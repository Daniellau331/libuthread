#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

/* Data structure for each thread */
typedef struct {
	uthread_t tid;
	uthread_ctx_t *context;
	void *stackPtr;
	int state; // 0-Running, 1-Ready, 2-Zombie, 3-Blocked
	int ret; // return value
	int joinThd; // TID of the thread to be joined by (-1 if none)
} TCB;

/* Global variables */
static queue_t queue = NULL;
static uthread_t TID = 1; // defined in uthread.h
static TCB *curr_TCB = NULL; // Ptr to current thread TCB


/* For debugging purpose: Prints all items in queue */
int queue_print(void *data, void *arg) {
	TCB *ptr = (TCB*)data;
	printf("TID %d & STATE %d\n", ptr->tid, ptr->state);
	return 0;
}

/* Find a TCB based on tid */
int find_TCB(void *data, void *arg) {
	TCB *ptr = (TCB*)data;
	uthread_t *thdID = (uthread_t*)arg;
	if (ptr->tid == *thdID)
		return 1;
	return 0;
}

/* Find a TCB with state 1-Ready */
int find_ready(void *data, void *arg) {
	TCB *ptr = (TCB*)data;
	if (ptr->state == 1)
		return 1;
	return 0;
}

/*  
 * Switch to next ready thread 
 * At the end, the running thread is at the end of queue
 */
void uthread_yield(void) {
	preempt_disable();
	TCB *running_thd = curr_TCB; 
	TCB *ready_thd = NULL;

	// if zombie or blocked, do not change state
	if (running_thd->state == 0) 
		running_thd->state = 1; // now ready

	// find ready thread and switch context
	// if queue length is 1 (only "main" thd), we are done 
	if (queue_iterate(queue, find_ready, NULL, (void**)&ready_thd) == 0 
		&& queue_length(queue) > 1) { 
		
		// repeat deq and enq until a ready thd is found
		queue_dequeue(queue, (void**) &ready_thd);
		while(ready_thd->state != 1) {
			queue_enqueue(queue, ready_thd); 
			queue_dequeue(queue, (void**) &ready_thd);
		}
		queue_enqueue(queue, ready_thd);
		
		ready_thd->state = 0; // ready thd becomes current, running thd
		curr_TCB = ready_thd;  
		
		uthread_ctx_switch(running_thd->context, ready_thd->context);
	} 
}

/* Return TID of the currently running thread */
uthread_t uthread_self(void) {
	return curr_TCB->tid;
}

/* Initialize the "main" thread */
int initialize(uthread_func_t func, void *arg) {
	queue = queue_create();
	
	// "main" thread does not have to be initialized (tid = 0)
	uthread_ctx_t curr_ctx;

	TCB *mainThd = malloc(sizeof(TCB));
	mainThd->tid = 0;  
	mainThd->context = malloc(sizeof(uthread_ctx_t));
	mainThd->context = &curr_ctx;
	mainThd->state = 1; // Or 1-Ready?
	mainThd->joinThd = -1;  

	queue_enqueue(queue, mainThd);
	curr_TCB = mainThd;

	// Then create the first user thread
	TCB *newThd = malloc(sizeof(TCB));
	newThd->tid = TID;
	newThd->state = 1; 
	newThd->joinThd = -1;

	newThd->context = malloc(sizeof(uthread_ctx_t));
	newThd->stackPtr = uthread_ctx_alloc_stack();
	if (uthread_ctx_init(newThd->context, newThd->stackPtr, func, arg) != 0)
		return -1;

	queue_enqueue(queue, newThd);
	TID++; 

	preempt_start(); 

	return 0;
}

/* Create a new thread */
int uthread_create(uthread_func_t func, void *arg) {
	// If first new thread (empty queue), initialize queue and main thread
	if(queue == NULL) {
		if (initialize(func, arg) == 0) {
			return 1; // the first thd's tid
		}
	}

	// Create a user thread
	TCB *newThd = malloc(sizeof(TCB));
	newThd->tid = TID;
	newThd->state = 1; 
	newThd->joinThd = -1; // no thread to join to at the moment

	newThd->context = malloc(sizeof(uthread_ctx_t));
	newThd->stackPtr = uthread_ctx_alloc_stack();
	if (uthread_ctx_init(newThd->context, newThd->stackPtr, func, arg) != 0)
		return -1;

	queue_enqueue(queue, newThd);
	TID++; // TID for next thread
	return newThd->tid;
}

/* Called by an active thread to finish its execution */
void uthread_exit(int retval) {
	preempt_disable();
	TCB *running_thd = curr_TCB;
	running_thd->state = 2; // now zombie state
	running_thd->ret = retval;
	queue_delete(queue, running_thd); // remove from queue. resouces are not freed
	
	// find join TCB and unblock
	if (running_thd->joinThd != -1) {
		TCB *joinTCB = NULL;
		int *jointhd = &curr_TCB->joinThd;
		if (queue_iterate(queue, find_TCB, (void*)jointhd, (void**)&joinTCB) == 0) {
			joinTCB->state = 1;
		}
	} 
	uthread_yield();
}

 

/* tid (child) joins calling thread (parent) */
int uthread_join(uthread_t tid, int *retval) {
	TCB *child_thd = NULL;
	TCB *parent_thd = curr_TCB; // <=> calling thread

	uthread_t *childtid = &tid;

	// prelimiary checks & find child_thd based on tid
	if (tid == 0)
		return -1; // main thread cannot be joined
	if (tid == uthread_self())
		return -1; // tid is same as calling thd
	if (queue_iterate(queue, find_TCB, (void*)childtid, (void**)&child_thd) == 0) {
		if (!child_thd)
			return -1; // if tid cannot be found
	}
	if (child_thd->joinThd != -1)
		return -1;  // if child is already being joined

	while(1) {
		if (child_thd->state == 0 || child_thd->state == 1) {
 			// If child is active, block parent until child dies
			parent_thd->state = 3; // block 
			child_thd->joinThd = uthread_self();
 			uthread_yield();

 		} else if (child_thd->state == 2) {
 			// Child now dead. Now collect and free
			if (child_thd->ret)
				*retval = child_thd->ret;
			free(child_thd); 
			break;
 		}
	} 
 	return 0;
}


