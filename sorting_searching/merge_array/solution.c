/* You are given two sorted arrays, A and B, where A has a large enough buffer at
 * the end to hold B. Write a method to merge B into A in sorted order.
 *
 * Source: Cracking the Coding Interview, page 121, question 11.1
 */
#include <stdio.h>

void merge(int arrayA[], size_t arrayA_sz, int arrayB[], size_t arrayB_sz) {
	ssize_t i, j, k;

	if (arrayA_sz == 0)
		i = -1;
	else
		i = arrayA_sz-1;

	if (arrayB_sz == 0)
		return;

	j = arrayB_sz-1;
	k = arrayA_sz+arrayB_sz-1;

	while (i >= 0 && j >= 0) {
		if (arrayA[i] > arrayB[j]) {
			arrayA[k] = arrayA[i];
			i--;
		} else {
			arrayA[k] = arrayB[j];
			j--;
		}
		k--;
	}

	while (j >= 0) {
		arrayA[k--] = arrayB[j--];
	}
}

void print_arr(int arr[], size_t arr_sz) {
	size_t i = 0;

	printf("[ ");

	if (arr_sz > 0) {
		printf("%d", arr[0]);
	}

	for (i = 1; i < arr_sz; i++) {
		printf(", %d", arr[i]);
	}

	printf(" ]");
}

int main(void) {
	printf("Enter A's size, followed by B's size, followed by the elements of A and then "
	       "the elements of B.\n");

	size_t a_sz, b_sz;
	while (scanf("%zu%zu", &a_sz, &b_sz) == 2) {
		int A[a_sz+b_sz];
		int B[b_sz];

		size_t i;
		for (i = 0; i < a_sz; i++) {
			scanf("%d", &A[i]);
		}
		for (i = 0; i < b_sz; i++) {
			scanf("%d", &B[i]);
		}

		merge(A, a_sz, B, b_sz);

		printf("Merged:\n");
		print_arr(A, a_sz+b_sz);
		printf("\n");
	}

	return 0;
}
