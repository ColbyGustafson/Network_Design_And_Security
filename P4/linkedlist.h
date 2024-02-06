#ifndef LINKEDLIST_H
#define LINKEDLIST_H

// Define the structure for a single node in the linked list
typedef struct Node {
    char* data;
    struct Node* next;
} Node;

// Define the structure for the linked list itself
typedef struct LinkedList {
    Node* head;
    int size;
} LinkedList;

// Function prototypes
LinkedList* createLinkedList();
void appendAtFront(LinkedList* list, const char* data);
void append(LinkedList* list, const char* data);
void removeNode(LinkedList* list);
void printLinkedList(LinkedList* list);
int isEmpty(LinkedList* list);
char* peek(LinkedList* list);
void destroyLinkedList(LinkedList* list);

#endif
