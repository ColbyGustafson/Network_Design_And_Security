#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

// Function to create an empty linked list
LinkedList* createLinkedList() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

void appendAtFront(LinkedList* list, const char* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    newNode->data = strdup(data); // Copy the string
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

void removeNode(LinkedList* list) {
    if (isEmpty(list)) {
        fprintf(stderr, "Cannot pop from an empty list\n");
        exit(1);
    }
    Node* temp = list->head;
    list->head = list->head->next;
    free(temp->data); // Free the string
    free(temp);
    list->size--;
}

// Function to print the elements of the linked list
void printLinkedList(LinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        printf("%s -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");
}

// Function to check if the linked list is empty
int isEmpty(LinkedList* list) {
    return list->size == 0;
}

// Function to free memory allocated for the linked list
void destroyLinkedList(LinkedList* list) {
    while (!isEmpty(list)) {
        removeNode(list);
    }
    free(list);
}

char* peek(LinkedList* list) {
    if (isEmpty(list)) {
        fprintf(stderr, "Cannot peek at an empty list\n");
        exit(1);
    }

    return list->head->data; // Return the data of the head (last element added)
}

void append(LinkedList* list, const char* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->data = strdup(data); // Copy the string
    newNode->next = NULL; // The new node will be the last node

    if (isEmpty(list)) {
        // If the list is empty, set the new node as the head
        list->head = newNode;
    } else {
        // Find the current last node and update its 'next' to point to the new node
        Node* current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }

    list->size++;
}
