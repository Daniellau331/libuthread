/*
 * Phase 3 testing
 * Tests uthread_join, which should wait for a thread's termination 
 * and collect its information
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include <unistd.h>

int thread2(void* arg)
{
	sleep(3);
	printf("thread 2 execution: returning value 5\n");
	return 5;
}

int thread1(void* arg)
{
	uthread_t tid;
	int retval;

	tid = uthread_create(thread2, NULL);
	uthread_join(tid, &retval);
	
	printf("thread 1 execution: waited and obtained return value %d from thread2\n", retval);
	return 0;
}

int main(void)
{
	uthread_t tid;
	tid = uthread_create(thread1, NULL);
	uthread_join(tid, NULL);

	return 0;
}