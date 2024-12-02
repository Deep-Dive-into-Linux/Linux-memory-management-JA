#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define ALLOC_SIZE (100 * 1024 * 1024)
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define PAGE_SIZE 4096 // 1 << 12

void print_stack_values()
{
	printf("-----------------------stack-----------------------\n");
	int32_t arrray[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	for (size_t i = 0; i < 10; i++) {
		printf("stack: %p: %d\n", &arrray[i], arrray[i]);
	}
	printf("-----------------------stack-----------------------\n");
	getchar();
}

int main(int argc, char const *argv[])
{
	print_stack_values();
	printf("-----------------------heap-----------------------\n");
	printf("Will allocate memory\n");

	char *heap = malloc(sizeof(char) * ALLOC_SIZE);
	if (heap == NULL) {
		fprintf(stderr, "failed to allocate memory\n");
		return 1;
	}
	printf("Succeeded to allocate memory: address = %p; size = 0x%x ***\n",
	       heap, ALLOC_SIZE);
	printf("Before write values to physical memory\n");
	getchar();

	int32_t unit_of_print_out = 10 * 1024 * 1024;
	time_t t;
	char *s;
	for (int32_t i = 0; i < ALLOC_SIZE; i += PAGE_SIZE) {
		heap[i] = 0;
		if (i != 0 && i % unit_of_print_out == 0) {
			t = time(NULL);
			s = ctime(&t);
			printf("%.*s: touched %dMB\n", (int)(strlen(s) - 1), s,
			       i / (1024 * 1024));
			sleep(1);
		}
	}

	free(heap);

	return 0;
}
