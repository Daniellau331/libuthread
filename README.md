+ ## Phase 1: Queue API
  + To implement the queue, we decided to use a doubly linked list. This was to  
    maintain the O(1) requirement for most operations. Using a linked list  
    allowed for O(1) enqueueing and dequeueing that could be difficult with  
    other data structures, and it matched the specifications perfectly.  
    The queue structure contained a pointer to its first and last nodes as well  
    as an int to track its length. The nodes of the queue contained a pointer  
    to a thread (TCB) member as well as pointers to their next and previous  
    nodes.
    ```
    typedef struct node_t {
    	void *value;
  	  struct node_t *next;
    	struct node_t *prev;
    } node_t;

    struct queue {
    	node_t *first;
    	node_t *last;
	    int length;
    };
    ```
  + To handle queue_create(), memory was simply allocated for the queue and if  
    allocation failed, NULL was returned. Otherwise, the queue's nodes were  
    initialized to NULL and its length to 0.
    
  + For enqueue(), cases in which the queue or data were NULL, malloc erred, the  
    queue was empty, or the queue contained one or more nodes were accounted  
    for. Most of the queue implementation involved basic pointer manipulations  
    such as the following.
    ```
    	newNode->value = data;
	    ...
	    } else {
    		// one or more items in queue
	    	newNode->next = NULL;
  		  newNode->prev = queue->last;
		    queue->last->next = newNode;
	    	queue->last = newNode;
    	}
    	queue->length++;
	    return 0;
    }
    ```
  + In order to satisfy the condition of queue_length being O(1) in time  
    complexity, queue length was simply incremented every time a node was  
    enqueued and returned in the queue_length() function.  

  + In the case of queue_delete(), the possibilities of the queue or data being  
    NULL, the queue being empty, the data not being found, and the queue having  
    contents to be deleted were handled. It compares the pointer addresses and  
    deletes the appropriate node once the match is found.
    
  + For queue_destroy(), a simple check was made to determine whether the queue  
    was not NULL or had a length of 0. If that held true, the queue was freed.  

  + To take care of queue_iterate(), it was first determined whether the queue  
    or function were NULL. If not, a pointer to the first node of the queue was  
    instantiated to serve as the iterator through the queue. The iterator goes  
    through each node, calling the function specified, and, depending on the  
    return value of that function, receives the data or not.

  + The implementation of queue_dequeue() was very similar to that of  
    queue_enqueue(). However, the use of a double pointer allowed us to save  
    the address to data.  

  + To test the queue, we created a test_queue.c file. This manipulated nodes  
    in the queue.  Each node consisted of two ints and two char pointers. Each  
    queue function was tested using either assert statements, strcmp, or prints  
    to stderr as shown below. Queue_iterate() was tested using the given code  
    under the "Hints" heading of the project2.html page. 
    ```
  	assert(queue_destroy(NULL) == -1);
  	assert(queue_enqueue(NULL, NULL) == -1);
  	myData data1 = {1, 2, "Sevilla", "Bayern"};
	  myData data2 = {0, 3, "Juventus", "Real Madrid"};
    ...
  	queue_t testq = queue_create();
  	void *data = NULL;
  	assert(queue_dequeue(testq, &data) == -1); //must return -1 (empty)
  	assert(queue_enqueue(testq, match1) == 0); // must return 0 (success)
    ...
	  if (strcmp(ret->homeTeam, "Juventus") || strcmp(ret->awayTeam, "Real Madrid")) {
  		fprintf(stderr, "%s\n", "Incorrect item dequeued");
      ...
  	}
    ...
  	int len = queue_length(testq);
  	if (len != 6) {
  		fprintf(stderr, "%s", "Incorrect queue length");
      ...
  	}
    ```
+ ## Phase 2: Uthread API
  + To implement the TCB structure, we used six variables to keep track of the  
    TID, context, stack pointer, state, return value, and the TID of the thread to  
    be joined. The state was an integer with 0, 1, 2, and 3 signifying Running,  
    Ready, Zombie, and Blocked states respectively.
    ```
    typedef struct {
    	uthread_t tid;
  	  uthread_ctx_t *context;
    	void *stackPtr;
  	  int state; // 0-Running, 1-Ready, 2-Zombie, 3-Blocked
     	int ret; // return value
    	int joinThd; // TID of the thread to be joined by (-1 if none)
    } TCB;
    ```
  + For ease of use in the subsequent functions specified, global variables the  
    queue, TID, and a pointer to the current thread were declared. We also created  
    a few utility functions as well. The function uthread_self() uses the global  
    variable to easily identify the current thread's thread ID. The functions  
    find_ready() and find_TCB() took advantage of the unique queue iteration  
    mechanism we implemented in the first phase. When used with  
    queue_iterate(), they will help locate the TCB based on a thread ID and the  
    TCB of the first ready thread, respectively.  
    ```  
    /* Global variables */
    static queue_t queue = NULL;
    static uthread_t TID = 1; // defined in uthread.h
    static TCB *curr_TCB = NULL; // Ptr to current thread TCB
    ...
    /* Find a TCB with state 1-Ready */
    int find_ready(void *data, void *arg) {
    	TCB *ptr = (TCB*)data;
  	  if (ptr->state == 1)
  	  	return 1;
    	return 0;
    }
    ```  
  + The uthread_create() function is in charge of creating a new thread using the  
    context API and enqueuing it as a ready thread. The context API simplifies  
    the task considerably. We initialize the variables and allocate space for  
    the stack and the context. The actual initialization of the execution  
    context of the thread is also done by the context API. The following code  
    snippet shows this process.  
    ```  
	  //Create a user thread
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
    ```  
  + In uthread_create(), when the queue is empty, our library recognizes that  
    it is the first library call from the user. Therefore, we use a separate  
    function initialize() to initalize the queue, the "main" thread, and the  
    first user thread. This process is similar to a typical thread creation  
    process described in uthread_create(). However, it is noteworthy that the  
    "main" thread does not have to be initialized since it already has a stack.  
    The preemption process, which we will describe in detail in Phase 4, also  
    begins at the end of this function. 

  + For uthread_exit(), the running thread's state is changed to Zombie, its  
    return value is collected, and queue_delete() is called to remove the  
    thread from the queue. Its resources, however, are not freed here because  
    they may be collected by the thread it is joining to. At the end of the  
    function, we make sure to unblock the thread it is planning to join to, if  
    present. (This part is implemented in Phase 3).  

  + In the function uthread_yield(), the current thread becomes ready unless it  
    is blocked. This ensures that an unblocking of a thread can only be done  
    when the child thread's execution exits. If the queue only contains one  
    element ("main" thread) or if there is no more ready thread, we are done.  
    Otherwise, we repeatedly enqueue and dequeue until we find the first ready  
    thread. Once found, its state is switched to running and is placed at the  
    end of the queue. We change the global variable for the current TCB  
    accordingly and switch the context using a context API function.
    
  + We used the given files to test this phase. 

+ ## Phase 3: uthread_join()
  + The uthread_join() function was created with the child joining the calling  
    parent. It was first determined if the main thread could not be joined, if  
    the TID was the same as the calling thread's, if the TID could not be found  
    using queue_iterate(), and if the child was already being joined.
    ```
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
    ```  
    Then, if the child was Ready or Running, the parent was blocked until the  
    child died. If the child was dead, then its return value was collected and  
    it was freed.
    ```
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
    ```
  + For testing, we created test_phase3.c. In this file, thread1 waits on  
    thread2 to terminate and collect its return value using uthread_join().
    ```
    int thread2(void* arg) {
    	sleep(3);
    	printf("thread 2 execution: returning value 5\n");
    	return 5;
    }
    
    int thread1(void* arg) {
    	uthread_t tid;
    	int retval;
    	tid = uthread_create(thread2, NULL);
    	uthread_join(tid, &retval);
    	printf("thread 1 execution: waited and obtained return value %d from thread2\n", retval);
    	return 0;
    }
    int main(void) {
    	uthread_t tid;
    	tid = uthread_create(thread1, NULL);
    	uthread_join(tid, NULL);
    	return 0;
    }
    ```
+ ## Phase 4: Preemption
  + The forth phase mainly involved standard implementations of timed alarms and  
    their handler. The function setitimer() sets an emission of the signal  
    SIGVTALRM a hundred times per second. The function sigaction() handles  
    these signals by calling a handler that simply calls uthread_yield(). The  
    functions preempt_{enable,disable} are also simple implementations of  
    signal blocking, following the GNU manual. This mechanism ensures that no  
    one thread will hoard all resources and prevent other threads from  
    executing.  

  + Preemption begins at the end of the function initialize(), which sets up the  
    "main" thread and the first user thread. To prevent preemption from causing  
    inconsistent program behaviors, we strictly disabled it in the yield process  
    and the exit process. On the other hand, by inserting the function  
    preempt_enable() in the bootstrap function, we made sure that preemption is  
    active during the execution of threads.  

  + We wrote a simple program called test_preempt.c to test this phase. Without  
    preemption, this programs gets stuck in thread1's infinite loop, and thread2  
    and thread3 are never executed. However, with preemption, although the  
    program eventually gets trapped in the infinite loops, all two print  
    statements from thread2 and thread3 are executed. This proves that threads  
    are regularly being forced to yield due to our implementation of preemption.  
	  ```
	  int thread3(void* arg) {
	  	printf("thread3 execution\n");
	  	return 0;
	  }
	  int thread2(void* arg) {
	  	printf("thread2 pause starting..\n");
	  	while(1);
	    	return 0;
	  }
	  int thread1(void* arg) {
	  	uthread_create(thread2, NULL);
	  	uthread_create(thread3, NULL);
	  	while(1);
	    	return 0;
	  }
	  int main(void) {
	  	uthread_t tid;
	  	tid = uthread_create(thread1, NULL);
		  uthread_join(tid, NULL);
  		return 0;
	  }
	  ```
## Sources:
+ https://lasr.cs.ucla.edu/vahab/resources/signals.html
+ https://stackoverflow.com/questions/4802038/implement-a-queue-in-which-push-rear-pop-front-and-get-min-are-all-consta
+ http://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15750-s03/www/amortized-analysis.txt
+ https://www.geeksforgeeks.org/doubly-linked-list/
+ http://faculty.cs.niu.edu/~freedman/241/241notes/241link.htm
+ https://www.geeksforgeeks.org/linked-list-set-2-inserting-a-node/
+ https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Actions
+ https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm
