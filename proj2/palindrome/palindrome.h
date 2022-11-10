#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Fowler-Noll-Vo hash constants, for 32-bit word sizes. */
#define FNV_32_PRIME 16777619u
#define FNV_32_BASIS 2166136261u

#define TRUE    1
#define FALSE   0 
#define CAPACITY 8

typedef struct NODE Node;
typedef struct HASHTABLE Hashtable;

unsigned long hash_function (const char *s_);

Node* create_node(char *key);
Hashtable* create_htable(int size);

void free_node(Node *node);
void free_htable(Hashtable *htable);

Node* ht_insert(Hashtable *htable, char *key);
int ht_search(Hashtable *htable, char *key);

void reverse_word(char *r_word, char *word);

struct NODE {
    char *key;
    Node *next;
};

struct HASHTABLE {
    Node **nodes;
    int size;
    int count;
};
