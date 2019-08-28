#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common.h>
#include <protocol.h>

#define N_OBJECTS       20		// Number of objects created
#define MIN_SIZE        100		// Minimum object size
#define DATANAME_SIZE   7		// Chars needed by the name "xx.txt"
#define K               4995	// (100000 - 100)/20

typedef struct {
	int num_success;
	int num_fails;
} client_stats;

client_stats _stats;


client_stats initStats()
{
	client_stats c_stats;
	c_stats.num_success = 0;
	c_stats.num_fails = 0;

	return c_stats;
}

void printStats()
{
	printf("%d %d %d\n", _stats.num_success + _stats.num_fails,
						 _stats.num_success,
						 _stats.num_fails);
}

void updateStats(int result)
{
	if (result == TRUE) {
		_stats.num_success++;
	} else {
		_stats.num_fails++;
	}
}

char valueAtIndex(size_t i)
{
	return (char)(127 - (i % 255));
}

void *createData(size_t len)
{
	char *data = (char *)calloc(len, sizeof(char));
	check_calloc(data, NULL);

	for (size_t i = 0; i < len; ++i) {
		data[i] = valueAtIndex(i);
	}
	return (void *)data;
}

int checkData(void *block, size_t len)
{
	if (block == NULL && len > 0)	return FALSE;
	char *data = (char *)block;

	for (size_t i = 0; i < len; ++i) {
		if (data[i] != valueAtIndex(i)) {
			return FALSE;
		}
	}
	return TRUE;
}

void makeTest1()
{
	char dataname[DATANAME_SIZE];

	for (size_t i = 0; i < N_OBJECTS; ++i) {
		size_t len = K * i + MIN_SIZE;
		void *data = createData(len);
		snprintf(dataname, DATANAME_SIZE, "%02zu.txt", i);

		updateStats(os_store(dataname, data, len));
	}
}

void makeTest2()
{
	char dataname[DATANAME_SIZE];

	for (size_t i = 0; i < N_OBJECTS; ++i) {
		snprintf(dataname, DATANAME_SIZE, "%02zu.txt", i);
		size_t len = K * i + MIN_SIZE;

		void *data = os_retrieve(dataname);
		updateStats(checkData(data, len));

		free(data);
		data = NULL;
	}
}

void makeTest3()
{
	char dataname[DATANAME_SIZE];

	for (size_t i = 0; i < N_OBJECTS; ++i) {
		snprintf(dataname, DATANAME_SIZE, "%02zu.txt", i);

		updateStats(os_delete(dataname));
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Not enough inputs");
		exit(EXIT_FAILURE);
	}

	argc--;
	argv++;

	char *name = argv[0];
	char *test_str = argv[1];
	int test_case = strtol(test_str, NULL, 10);

	_stats = initStats();

	if (os_connect(name) == FALSE) {
		_stats.num_fails++;
		printStats();
		exit(EXIT_FAILURE);
	}
	_stats.num_success++;

	switch (test_case) {

		case 1 :		// Store
			makeTest1();
			break;

		case 2 :		// Retrieve
			makeTest2();
			break;

		case 3 :		// Delete
			makeTest3();
			break;

		default:
			break;
	}

	os_disconnect();
	_stats.num_success++;
	printStats();
	return 0;
}
