/* You are given an array of N unique integers such that each integer i
 * satisfies 0 <= i < 2n.
 *
 * Write a program that prints every integer x, where 0 <= x < 2n, such that x
 * does not occur in the array.
 *
 * Strive for O(n) time complexity and O(1) space complexity.
 *
 * EXAMPLE
 *
 * Input:
 *
 * [ 0, 2, 4 ]
 *
 * Output:
 * [ 1, 3, 5 ]
 *
 * Explanation:
 *
 * We have n = 3, so numbers are in the range [0..5].
 * The numbers in the array are [ 0, 2, 4 ], so the missing numbers in the range
 * are [ 1, 3, 5 ].
 *
 * Source: Careercup (Google interview)
 */

#include <stdio.h>

void swap(int a[], size_t i, size_t j) {
	int tmp = a[i];
	a[i] = a[j];
	a[j] = tmp;
}

void find_missing(int arr[], size_t len) {
	size_t i;

	for (i = 0; i < len; i++)
		if (arr[i] < len)
			swap(arr, i, arr[i]);

	for (i = 0; i < len; i++)
		if (arr[i] != i)
			printf("%zu ", i);

	for (i = 0; i < len; i++)
		if (arr[i] >= len)
			swap(arr, i, arr[i]-len);

	for (i = 0; i < len; i++)
		if (i+len != arr[i])
			printf("%zu ", i+len);
	putchar('\n');
}

int main(void) {
	printf("Enter a value N, followed by N array elements in the range [0..2N-1]\n> ");

	size_t len;
	while (scanf("%zu", &len) == 1) {
		int array[len];
		size_t i;
		for (i = 0; i < len; i++)
			scanf("%d", &array[i]);
		find_missing(array, len);
		printf("> ");
	}

	return 0;
}
