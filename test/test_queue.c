// test_queue.c
// Tests the queue implementation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../libuthread/queue.h"

typedef struct {
	int homeScore;
	int awayScore;
	char *homeTeam;
	char *awayTeam;
} myData;

int main() {
	
	// preliminary tests
	assert(queue_destroy(NULL) == -1);
	assert(queue_enqueue(NULL, NULL) == -1);
	
	myData data1 = {1, 2, "Sevilla", "Bayern"};
	myData data2 = {0, 3, "Juventus", "Real Madrid"};
	myData data3 = {3, 0, "Liverpool", "Man City"};
	myData data4 = {4, 1, "Barcelona", "Roma"};
	myData data5 = {1, 2, "Man City", "Liverpool"};
	myData data6 = {3, 0, "Roma", "Barcelona"};
	myData data7 = {0, 0, "Bayern", "Sevilla"};
	myData data8 = {1, 3, "Real Madrid", "Juventus"};
	
	myData *match1 = &data1;
	myData *match2 = &data2;
	myData *match3 = &data3;
	myData *match4 = &data4;
	myData *match5 = &data5;
	myData *match6 = &data6;
	myData *match7 = &data7;
	myData *match8 = &data8;
	
	//assert(queue_create() == NULL); // returns NULL if failure to allocate
	queue_t testq = queue_create();
	void *data = NULL;
	
	assert(queue_dequeue(testq, &data) == -1); //must return -1 (empty)
	assert(queue_enqueue(testq, match1) == 0); // must return 0 (success)
	assert(queue_enqueue(testq, match2) == 0);
	assert(queue_enqueue(testq, match3) == 0);
	assert(queue_enqueue(testq, match4) == 0);
	assert(queue_dequeue(testq, &data) == 0); // dequeue match1
	
	assert(queue_dequeue(testq, &data) == 0); // dequeue match2
	myData *ret = (myData*) data;
	// check if the right item has been dequeued
	if (strcmp(ret->homeTeam, "Juventus") || strcmp(ret->awayTeam, "Real Madrid")) {
		fprintf(stderr, "%s\n", "Incorrect item dequeued");
		fprintf(stderr, "%s\n", "Instead of Juventus and Real Madrid,");
		fprintf(stderr, "%s and %s\n\n\n", ret->homeTeam, ret->awayTeam);
	}
	
	// enqueue every item (order: 34125678)
	assert(queue_enqueue(testq, match1) == 0);
	assert(queue_enqueue(testq, match2) == 0);
	assert(queue_enqueue(testq, match5) == 0);
	assert(queue_enqueue(testq, match6) == 0);
	assert(queue_enqueue(testq, match7) == 0);
	assert(queue_enqueue(testq, match8) == 0);
	
	assert(queue_delete(testq, match4) == 0); // delete match4
	assert(queue_dequeue(testq, &data) == 0); // dequeue match3
	ret = (myData*) data; // ptr to match3
	// check if the right item has been dequeued
	if (strcmp(ret->homeTeam, "Liverpool") || strcmp(ret->awayTeam, "Man City")) {
		fprintf(stderr, "%s\n", "Incorrect item dequeued");
		fprintf(stderr, "%s\n", "Instead of Liverpool and Man City,");
		fprintf(stderr, "%s and %s\n\n\n", ret->homeTeam, ret->awayTeam);
	}
	
	// testing length function
	int len = queue_length(testq);
	if (len != 6) {
		fprintf(stderr, "%s", "Incorrect queue length");
		fprintf(stderr, "%s\n", "Instead of 6,");
		fprintf(stderr, "%d\n\n\n", len);
	}
	
	// empty out the queue and destroy
	assert(queue_dequeue(testq, data) == 0);
	assert(queue_dequeue(testq, data) == 0);
	assert(queue_dequeue(testq, data) == 0);
	assert(queue_dequeue(testq, data) == 0);
	assert(queue_dequeue(testq, data) == 0);
	assert(queue_dequeue(testq, data) == 0);
	assert(queue_destroy(testq) == 0); // must return 0 (success)
	
	return 0;
}

