
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

struct channel {
	sem_t reader_to_writer;
	sem_t writer_to_reader;
	sem_t data_writer;
	sem_t data_reader;

	/* A quick and simple simulation of Go slices in C to pass data around.
	 * The `data` pointer is the actual buffer; `len` is the number of bytes written
	 * in the buffer (that will be fetched by channel_read()), and `cap` is the buffer's
	 * capacity. The buffer is realloced if a write exceeds the size of the buffer.
	 */
	struct {
		char *data;
		size_t len;
		size_t cap;
	} data;
};

typedef struct channel channel_t;

channel_t *make_channel(void) {
	channel_t *ch = malloc(sizeof(*ch));
	if (ch == NULL)
		goto freem;
	if (sem_init(&ch->reader_to_writer, 0, 0) == -1)
		goto freem;
	if (sem_init(&ch->writer_to_reader, 0, 0) == -1)
		goto freer;
	if (sem_init(&ch->data_writer, 0, 1) == -1)
		goto freew;
	if (sem_init(&ch->data_reader, 0, 0) == -1)
		goto freedw;

	ch->data.data = NULL;
	ch->data.len = 0;
	ch->data.cap = 0;

	return ch;

freedw:
	sem_destroy(&ch->data_writer);
freew:
	sem_destroy(&ch->writer_to_reader);
freer:
	sem_destroy(&ch->reader_to_writer);
freem:
	free(ch);
	return NULL;
}

void destroy_channel(channel_t *ch) {
	sem_destroy(&ch->reader_to_writer);
	sem_destroy(&ch->writer_to_reader);
	free(ch);
}

int channel_write(channel_t *ch, char *data, size_t len) {

	if (len == 0) {
		errno = EINVAL;
		return -1;
	}

	if (sem_post(&ch->reader_to_writer) == -1)
		return -1;
	sem_wait(&ch->writer_to_reader);

	sem_wait(&ch->data_writer);
	if (ch->data.cap < len) {
		char *new_buf = realloc(ch->data.data, len*2);
		/* We handle allocation errors by setting the length to 0 and returning an error
		 * back to the caller.
		 *
		 * As zero-length reads and writes are disallowed, a length of 0 in channel_read()
		 * means that the corresponding channel_write() had an allocation error.
		 */
		if (new_buf == NULL) {
			ch->data.len = 0;
			sem_post(&ch->data_reader);
			errno = ENOMEM;
			return -1;
		}
		ch->data.data = new_buf;
		ch->data.cap = len*2;
	}

	memcpy(ch->data.data, data, len);
	ch->data.len = len;
	sem_post(&ch->data_reader);

	return 0;
}

int channel_read(channel_t *ch, char *data, size_t len) {

	if (len == 0) {
		errno = EINVAL;
		return -1;
	}

	if (sem_post(&ch->writer_to_reader) == -1)
		return -1;
	sem_wait(&ch->reader_to_writer);

	sem_wait(&ch->data_reader);
	size_t copy_len = len;
	if (ch->data.len < copy_len)
		copy_len = ch->data.len;

	if (copy_len == 0) {
		/* If we get here, realloc(3) failed on channel_write(), there's not much we can do
		 * at this point.
		 */
		sem_post(&ch->data_writer);
		errno = ENOMEM;
		return -1;
	}

	memcpy(data, ch->data.data, copy_len);
	sem_post(&ch->data_writer);

	return 0;
}

#define THREADS_N 10
#define COUNT 100

channel_t *chan;

static pthread_t threads[THREADS_N];

void *thr_fn(void *id) {
	int i;

	for (i = (uintptr_t) id; i < COUNT; i += THREADS_N) {
		if (channel_write(chan, (char *) &i, sizeof(i)) == -1) {
			perror("channel_write() failed");
			return NULL;
		}
	}

	return NULL;
}

int main(void) {

	if ((chan = make_channel()) == NULL) {
		perror("make_channel() error");
		exit(EXIT_FAILURE);
	}

	size_t i;
	for (i = 0; i < THREADS_N; i++) {
		int thr_res = pthread_create(&threads[i], NULL, thr_fn, (void *) (uintptr_t) i);
		if (thr_res != 0) {
			fprintf(stderr, "pthread_create(3) error: %s\n", strerror(thr_res));
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < COUNT; i++) {
		int j;
		if (channel_read(chan, (char *) &j, sizeof(j)) == -1) {
			perror("channel_read() error");
			exit(EXIT_FAILURE);
		}
		printf("%d\n", j);
	}

	return 0;
}
