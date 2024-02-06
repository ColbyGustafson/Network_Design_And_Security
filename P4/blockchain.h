#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include "linkedlist.h"

typedef struct block{
    char* hash_of_previous_block;
    char* transactions;
} Block;

char* simpleHash(const char* str);
char* generateBlockSummary(Block* block); 
void insertBlock(LinkedList* blockchain, char* transactions); 
char* generateHashOfPreviousBlock(LinkedList* blockchain);

#endif
