/*
 * list_mutex.h
 *
 *  Created on: 18/6/2016
 *      Author: utnso
 */

#ifndef COMMONS_COLLECTIONS_LIST_MUTEX_H_
#define COMMONS_COLLECTIONS_LIST_MUTEX_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <commons/collections/list.h>

typedef struct {
	t_list *list;
	pthread_mutex_t mutex;
} t_list_mutex;

t_list_mutex *list_mutex_create();
int list_mutex_add(t_list_mutex *self, void *data);
void* list_mutex_get(t_list_mutex *self, int index);
void *list_mutex_remove(t_list_mutex *self, int index);
int list_mutex_size(t_list_mutex *self);
int list_mutex_is_empty(t_list_mutex *list);
void list_mutex_destroy(t_list_mutex *self);

void list_mutex_clean(t_list_mutex *self);
void list_mutex_iterate(t_list_mutex* self, void(*closure)(void*));
void list_mutex_destroy_and_destroy_elements(t_list_mutex *self, void(*element_destroyer)(void*));
void list_mutex_clean_and_destroy_elements(t_list_mutex *self, void(*element_destroyer)(void*));


#endif /* COMMONS_COLLECTIONS_LIST_MUTEX_H_ */
