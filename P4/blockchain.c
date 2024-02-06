// CLASS: 4300 - Perouli
// PROJECT: 4
// STUDENTS: Colby Gustafson & Timi Ladeinde

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blockchain.h"
#include <openssl/sha.h>
#include <openssl/evp.h>

// Generates hash value of a string based on standard SHA256 algorithm
//
char* sha256(const char* input) 
{
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    size_t data_len = strlen(input);

    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname("sha256");

    if(!md)
    {
        perror("Unknown digest used");
    }
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, data_len);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_DigestFinal_ex(mdctx, hash, NULL);
    EVP_MD_CTX_free(mdctx);
    EVP_cleanup();

    char *hex_hash = (char*)malloc(2*SHA256_DIGEST_LENGTH + 1);
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(&hex_hash[i*2], "%02x", hash[i]);
    }
    hex_hash[2 * SHA256_DIGEST_LENGTH] = '\0';
    return hex_hash;
}

/* Simple hash that converts strings to an integer value and 
 * returns the integer value as a string.
 * */
char* simpleHash(const char* str) 
{
    return sha256(str);
}

/* TODO
 * Creates a block object that contains the hash of the previous block 
 * along with the transactions...
 * */
Block* populateBlock(char* hash_of_previous_block, char* transactions)
{
    Block* blockToPopulate;
    blockToPopulate = malloc(sizeof(Block));

    strcpy(blockToPopulate->hash_of_previous_block, hash_of_previous_block);
    strcpy(blockToPopulate->transactions, transactions);
    return blockToPopulate;
}


/* TODO
 * Generates a string representation of the block by combining the hash 
 * with the transactions as a string...
 * */
char* generateBlockSummary(Block* block) 
{
    size_t lenHash = strlen(block->hash_of_previous_block);
    size_t lenTrans = strlen(block->transactions);

    char* summary = (char*) malloc(lenHash + lenTrans + 1);

    if (summary) 
    {
        memcpy(summary, block->hash_of_previous_block, lenHash);
        memcpy(summary + lenHash, block->transactions, lenTrans);

        summary[lenHash + lenTrans] = '\0';
    }
    return summary;
}


/* TODO
 * Inserts the block summary into the linked list...
 * */
void insertBlock(LinkedList* blockchain, char* transactions) 
{
    //nullcheck
    if (!blockchain) 
    {
        perror("Blockchain is NULL");
        return;
    }
    
    //init new node
    Node* newNode = calloc(1, sizeof(Node));
    if (!newNode) 
    {
        perror("Failed to allocate memory for new node");
        return;
    }

    newNode->data = transactions;

    //insert new node
    if (blockchain->head == NULL) 
    {
        blockchain->head = newNode; //add to beginning
    } 
    else 
    {
        Node* current = blockchain->head; //add to end
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = newNode;
    }
}


/* TODO
 * Generates a hash of the previous block using the block summary contents...
 * */
char* generateHashOfPreviousBlock(LinkedList* blockchain) 
{
    // nullcheck+
    if (!blockchain || !(blockchain->head)) 
    {
        return NULL;
    }

    // navigates to last block
    Node* currentB = blockchain->head;
    while (currentB->next != NULL) 
    {
        currentB = currentB->next;
    }

    // generate hash
    char* lastBData = currentB->data;
    char* curHash = simpleHash(lastBData);

    return curHash;
}
