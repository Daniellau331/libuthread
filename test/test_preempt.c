/*
 * Tests phase 4, an implementation of preemption
 * All printf statements must be executed despite the presence
 * of infinite loops
 */

#include <stdio.h>
#include <stdlib.h>
#include <uthread.h> 

int thread3(void* arg)
{
	printf("thread3 execution\n");
	return 0;
}

int thread2(void* arg)
{
	printf("thread2 pause starting..\n");
	while(1); 
	return 0;
}

int thread1(void* arg)
{
	uthread_create(thread2, NULL);
	uthread_create(thread3, NULL);
	while(1);
	return 0;
}

int main(void)
{
	uthread_t tid;
	tid = uthread_create(thread1, NULL);
	uthread_join(tid, NULL);
	
	return 0;
}
