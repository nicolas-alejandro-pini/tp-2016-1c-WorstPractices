/*
 * list_mutex.c
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */
#include "list_mutex.h"

#include <unistd.h>

t_list_mutex *list_mutex_create(){

	t_list_mutex *list_mutex = malloc(sizeof(t_list_mutex));

	list_mutex->list = list_create();
	pthread_mutex_init(&(list_mutex->mutex), NULL);

	return list_mutex;
}

int list_mutex_add(t_list_mutex *self, void *data) {
	t_list *list;
	int elements_count;

	pthread_mutex_lock(&self->mutex);
	list = self->list;
	elements_count = list_add(list, data);
	pthread_mutex_unlock(&self->mutex);

	return elements_count;
}

void* list_mutex_get(t_list_mutex *self, int index) {
	t_list *list;
	void *data = NULL;

	pthread_mutex_lock(&self->mutex);
	if(index >= 0 && index < self->list->elements_count){
		list = self->list;
		data = list_get(list, index);
	}
	pthread_mutex_unlock(&self->mutex);

	return data;
}

void *list_mutex_remove(t_list_mutex *self, int index) {
	void *data;
	t_list *list;

	pthread_mutex_lock(&self->mutex);
	if(index >= 0 && index < self->list->elements_count){
		list = self->list;
		data = list_remove(list, index);
	}
	pthread_mutex_unlock(&self->mutex);

	return data;
}

int list_mutex_size(t_list_mutex *self) {
	t_list *list;
	int elements_count;

	pthread_mutex_lock(&self->mutex);
	list = self->list;
	elements_count = list_size(list);
	pthread_mutex_unlock(&self->mutex);

	return elements_count;
}

int list_mutex_is_empty(t_list_mutex *list) {
	return list_mutex_size(list) == 0;
}

void list_mutex_destroy(t_list_mutex *self) {
	t_list *list;

	pthread_mutex_lock(&self->mutex);
	list = self->list;
	list_destroy(list);
	pthread_mutex_unlock(&self->mutex);
	free(self);

}

void list_mutex_clean(t_list_mutex *self) {

	pthread_mutex_lock(&self->mutex);
	list_clean(self->list);
	pthread_mutex_unlock(&self->mutex);
}

void list_mutex_clean_and_destroy_elements(t_list_mutex *self, void(*element_destroyer)(void*)){
	list_mutex_iterate(self, element_destroyer);
	list_mutex_clean(self);
}

void list_mutex_destroy_and_destroy_elements(t_list_mutex *self, void(*element_destroyer)(void*)) {
	list_mutex_clean_and_destroy_elements(self, element_destroyer);
	free(self);
}

void list_mutex_iterate(t_list_mutex* self, void(*closure)(void*)){

	pthread_mutex_lock(&self->mutex);
	list_iterate(self->list, closure);
	pthread_mutex_unlock(&self->mutex);
}


