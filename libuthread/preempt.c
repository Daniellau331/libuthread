#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

void preempt_disable(void)
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);
}

void preempt_enable(void)
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void handler(int sig) 
{
	uthread_yield(); 
}

void preempt_start(void)
{
	struct sigaction sa;
	struct itimerval timer;

	sa.sa_handler = handler;

	// configure itimerval
	timer.it_value.tv_sec = 0;  
	timer.it_value.tv_usec = 1000000 / HZ; // in microseconds
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000000 / HZ;

	if (sigaction(SIGVTALRM, &sa, NULL) == -1) {
		perror("Error in sigaction()");
		exit(1);
	}
	
	if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
    	perror("Error in setitimer()");
    	exit(1);
 	}

	while(1);
}



 




