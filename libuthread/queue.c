#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

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


queue_t queue_create(void) {
	queue_t queue = malloc(sizeof(struct queue));
	if (!queue)
		return NULL; // in case of allocation failure
	
	queue->first = NULL;
	queue->last = NULL;
	queue->length = 0;
	return queue;
}

int queue_destroy(queue_t queue) {
	//if queue is not empty or NULL
	if (!queue || queue->length != 0)
		return -1;
	
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data) {
	if(!queue || !data) // if @queue or @data are NULL
		return -1;
	
	node_t *newNode = malloc(sizeof(node_t));
	if (!newNode) // if malloc error
		return -1;
	
	newNode->value = data;
	
	if (!queue->first) {
		//if queue is empty, add node
		newNode->next = NULL;
		newNode->prev = NULL;
		queue->first = newNode;
		queue->last = newNode;
		
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

int queue_dequeue(queue_t queue, void **data) {
	if(!queue || !data) // if @queue or @data are NULL
		return -1;
	if(queue->length == 0) // if queue is empty
		return -1;
	
	// dequeue process
	if (queue->length == 1) {
		// only 1 node in queue
		node_t *toDeq = queue->first;
		free(toDeq);
		queue->first = NULL;
		queue->last = NULL;
	} else {
		// two or more nodes in queue
		node_t *toDeq = queue->first;
		*data = toDeq->value;
		queue->first->next->prev = NULL;
		queue->first = queue->first->next;
		free(toDeq);
	}
	queue->length--;
	return 0;
}

int queue_delete(queue_t queue, void *data) {
	if(!queue || !data) // if @queue or @data are NULL
		return -1;
	if(queue->length == 0) // if queue is empty
		return -1;
	
	int i = 0;
	node_t *itr = queue->first; // iterator
	while (itr) {
		if (itr->value == data) {
			// if first or last node
			if (i == 0) {
				if (itr->next)
					queue->first = itr->next;
			} else if (i == queue->length-1) {
				queue->last = itr->prev;
			}
			// delete the node then return 0
			if (itr->prev)
				itr->prev->next = itr->next;
			if (itr->next)
				itr->next->prev = itr->prev;
			free(itr);
			queue->length--;
			return 0;
		}
		itr = itr->next;
		i++;
	}
	return -1; // if @data was not found
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data) {
	if(!queue || !func) // if @queue or @func are NULL
		return -1;
	
	node_t *itr = queue->first; // iterator
	while (itr) {
		// if 1 is returned, the iteration stops
		if ( (*func)(itr->value, arg) == 1) {
			// if @data is not NULL, @data receives the item
			if (itr->value) {
				*data = itr->value; 
			}
			return 0;
		}
		itr = itr->next;
	}
	return 0;
}

int queue_length(queue_t queue) {
	if (queue)
		return queue->length;
	return -1; //if queue is NULL
}


