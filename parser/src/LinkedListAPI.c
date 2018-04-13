#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"

#include "LinkedListAPI.h"


List initializeList(char* (*printFunction)(void *toBePrinted),void (*deleteFunction)(void *toBeDeleted),int (*compareFunction)(const void *first,const void *second))
{
    List list;
    assert(printFunction != NULL);
    assert(deleteFunction != NULL);
    assert(compareFunction != NULL);
    list.head = NULL;
    list.tail = NULL;
    list.length = 0;
    list.deleteData = deleteFunction;
	list.compare = compareFunction;
	list.printData = printFunction;
    return list;

}

Node *initializeNode(void *data)
{
    Node* node = malloc(sizeof(Node));
    node->data = data;
    node->previous = NULL;
    node->next = NULL;
    return node;
}

void insertFront(List *list, void *toBeAdded)
{
    if (list == NULL || toBeAdded == NULL){
		return;
	}

    Node *newNode = initializeNode(toBeAdded);
    newNode->next = list->head;
    if (list->head != NULL) {
        list->head->previous = newNode;
    }

    list->head = newNode;

    if (list->tail == NULL) {
        list->tail = list->head;
    }

    list->length += 1;

    return;
}

void insertBack(List *list, void *toBeAdded)
{
    if (list == NULL || toBeAdded == NULL){
		return;
	}

    Node *newNode = initializeNode(toBeAdded);
    newNode->previous = list->tail;
    if (list->tail != NULL) {
        list->tail->next = newNode;
    }

    list->tail = newNode;
    if (list->head == NULL) {
        list->head = list->tail;
    }
    list->length += 1;
    return;
}

void clearList(List *list)
{
    if (list == NULL){
		return;
	}
	
	if (list->head == NULL && list->tail == NULL){
		return;
	}

    while (list->head != NULL) {
        void *deleteData = list->head->data;
        list->head = list->head->next;
        list->deleteData(deleteData);   
    }
    list->head = NULL;
	list->tail = NULL;
    free(list);
}

void *getFromFront(List list) {

    if (list.head == NULL) {
        return NULL;
    }
    void *dataStruct = list.head->data;
    return dataStruct;
}

void *getFromBack(List list) {

    if (list.tail == NULL) {
        return NULL;
    }
    void *dataStruct = list.tail->data;
    return dataStruct;
}

void insertSorted(List *list, void *toBeAdded) {

    if ((list == NULL) || (toBeAdded == NULL)) {
        return;
    }

    if (list->head == NULL) {
        insertFront(list, toBeAdded);
        return;
    }

    Node* node = list->head;
    
    while (node != NULL) {
    
        Node* tempNode = initializeNode(toBeAdded);
        if ((list->compare(tempNode->data, node->data)) < 0) {
            tempNode->previous = node->previous;
            tempNode->next = node;
            node->previous = tempNode;
            if (tempNode->previous == NULL) {
                insertFront(list, toBeAdded);
                return;
            }
            else {
                tempNode->previous->next = tempNode;
                list->head = list->head;
                return;
            }
        }
        if (node->next == NULL) {
            insertBack(list, toBeAdded);
            break;
        }
        node = node->next;
    }
    list->length += 1;
    return;
}
    

void* deleteDataFromList(List *list, void *toBeDeleted) {

    if ((list == NULL) || (list->head == NULL) || (toBeDeleted == NULL)) {
        return NULL;
    }

    Node* node = list->head;
    while (node != NULL) {
        if (list->compare(node->data, toBeDeleted) == 0){
            if (node->previous == NULL) {
                list->head = node->next;
                if (list->head != NULL) {
                    list->head->previous = NULL;
                }
            }
            else if (node->next == NULL) {
                list->tail = node->previous;
                if (node->previous != NULL) {
                    node->previous->next = NULL;
                }
            }
            else {
                node->previous->next = node->next;
                node->next->previous = node->previous;
            }

            list->deleteData(node->data);
            node->next = NULL;
            node->previous = NULL;
            free(node);
            list->length -= 1;
            return toBeDeleted;

        }
        node = node->next;
    }
    return NULL;
}

int getLength(List list) {

    if (list.length == -1) {
        return -1;
    }

    else {
        return list.length;
    }
}

void* findElement(List list, bool (*customCompare)(const void* first,const void* second), const void* searchRecord) {
    ListIterator iter = createIterator(list);
	void* elem;

	while( (elem = nextElement(&iter)) != NULL){
        
        if (customCompare(elem, searchRecord)) {
            return elem;
        }
    }
    return NULL;
}

/**Returns a string that contains a string representation of the list traversed from  head to tail. 
Utilize an iterator and the list's printData function pointer to create the string.
returned string must be freed by the calling function.
 *@pre List must exist, but does not have to have elements.
 *@param list Pointer to linked list dummy head.
 *@return on success: char * to string representation of list (must be freed after use).  on failure: NULL
 **/
char* toString(List list){

	ListIterator iter = createIterator(list);
	char* str;
		
	str = (char*)malloc(sizeof(char));
	strcpy(str, "");
	void* elem;
	while( (elem = nextElement(&iter)) != NULL){
		char* currDescr = list.printData(elem);
		int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc(str, newLen);
		strcat(str, "\n");
		strcat(str, currDescr);
		
		free(currDescr);
	}
	
	return str;
}

ListIterator createIterator(List list){
    ListIterator iter;
	
    iter.current = list.head;
    
    return iter;
}

void* nextElement(ListIterator* iter){
    Node* tmp = iter->current;
    
    if (tmp != NULL){
        iter->current = iter->current->next;
        return tmp->data;
    }else{
        return NULL;
    }
}


