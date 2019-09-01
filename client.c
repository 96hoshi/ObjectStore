#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <common.h>
#include <protocol.h>

#define N_OBJECTS		20				// Number of objects created
#define MAX_SIZE		100000			// Maximum object size
#define MIN_SIZE		100				// Minimum object size
#define MAX_SIZE_INT	(MAX_SIZE / 4)	// Maximum object size
#define MIN_SIZE_INT	(MIN_SIZE / 4)	// Minimum object size
#define DATANAME_SIZE	7				// Chars needed by the name "xx.txt"
#define K				1315			// ceil( ((100000 - 100) / sizeof(uint32_t))/(20 -1) )

typedef struct {
	int num_success;
	int num_fails;
} client_stats;

client_stats _stats;


void printStats()
{
	printf("%d %d %d\n", _stats.num_success + _stats.num_fails,
						 _stats.num_success,
						 _stats.num_fails);
}

void updateStats(int result, int test)
{
	if (result == TRUE) {
		_stats.num_success++;
	} else {
		_stats.num_fails++;
		printf("Test%d: ", test);
		printStats();
		exit(EXIT_FAILURE);
	}
}

// ritorna il numero di uint32_t che devo mandare/ricevere
size_t lenAtIndex(size_t i)
{
	size_t len = K * i + MIN_SIZE_INT;
	return (len > MAX_SIZE_INT ? MAX_SIZE_INT : len);
}

void *createData(size_t len)
{
	uint32_t *data = (uint32_t *)calloc(len, sizeof(uint32_t));
	if (data == NULL) return NULL;

	for (uint32_t i = 0; i < len; ++i) {
		data[i] = i;
	}

	return (void *)data;
}

int checkData(void *block, size_t len)
{
	uint32_t *data = (uint32_t *)block;

	for (uint32_t i = 0; i < len; ++i) {
		if (data[i] != i) {
			return FALSE;
		}
	}

	return TRUE;
}

int makeTest1()
{
	char dataname[DATANAME_SIZE];

	for (size_t i = 0; i < N_OBJECTS; ++i) {
		snprintf(dataname, DATANAME_SIZE, "%02zu.txt", i);

		size_t len = lenAtIndex(i);
		void *data = createData(len);
		if (data == NULL) {
			_stats.num_fails++;
			continue;
		}

		int result = os_store(dataname, data, len * sizeof(uint32_t));
		free(data);

		if (result == FALSE) {
			_stats.num_fails++;
			return FALSE;
		}
		_stats.num_success++;
	}

	return TRUE;
}

int makeTest2()
{
	char dataname[DATANAME_SIZE];

	for (size_t i = 0; i < N_OBJECTS; ++i) {
		snprintf(dataname, DATANAME_SIZE, "%02zu.txt", i);

		size_t len = lenAtIndex(i);
		void *data = os_retrieve(dataname);

		if (data == NULL) {
			_stats.num_fails++;
			return FALSE;
		}

		if (checkData(data, len) == TRUE) {
			_stats.num_success++;
		} else {
			_stats.num_fails++;
		}

		free(data);
	}

	return TRUE;
}

int makeTest3()
{
	char dataname[DATANAME_SIZE];

	for (size_t i = 0; i < N_OBJECTS; ++i) {
		snprintf(dataname, DATANAME_SIZE, "%02zu.txt", i);

		if (os_delete(dataname) == FALSE) {
			_stats.num_fails++;
			return FALSE;
		}
		_stats.num_success++;
	}

	return TRUE;
}

int main(int argc, char *argv[])
{
	_stats.num_success = 0;
	_stats.num_fails = 0;

	if (argc < 3) {
		fprintf(stderr, "Not enough inputs");
		exit(EXIT_FAILURE);
	}

	argc--;
	argv++;

	char *name = argv[0];
	char *test_str = argv[1];
	int test_case = strtol(test_str, NULL, 10);

	if (os_connect(name) == FALSE) {
		_stats.num_fails++;
		printf("Test%d: ", test_case);
		printStats();
		exit(EXIT_FAILURE);
	}
	_stats.num_success++;

	int result = FALSE;

	switch (test_case) {

		case 1 :		// Store
			result = makeTest1();
			break;

		case 2 :		// Retrieve
			result = makeTest2();
			break;

		case 3 :		// Delete
			result = makeTest3();
			break;

		default:
			break;
	}

	if (result == TRUE) {
		os_disconnect();
		_stats.num_success++;
	}

	printf("Test%d: ", test_case);
	printStats();

	return 0;
}
