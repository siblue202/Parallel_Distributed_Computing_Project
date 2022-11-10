#include "palindrome.h"

// argv[1] : num of thread
// argv[2] : input_file
// argv[3] : output_file

int main(int argc, char *argv[]) {
    omp_set_dynamic(0);
    int num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);

    double start, input_time, search_time, output_time;
    
    FILE *input, *output;
    char input_file[256];
    char output_file[256];
    strcpy(input_file, argv[2]);
    strcpy(output_file, argv[3]);

    start = omp_get_wtime();    
    input = fopen(input_file, "r");
    if (input == NULL) {
        printf("Error opening input_file");
    }

    Hashtable *hash_table = create_htable(CAPACITY);

    ssize_t read;
    char *tmp_line = NULL;
    size_t len = 0;
    while((read = getline(&tmp_line, &len, input)) != -1) {
        tmp_line[read-1] = '\0';
        ht_insert(hash_table, tmp_line);
    }
    free(tmp_line);

    input_time = omp_get_wtime();

    Node** find_node = (Node **)calloc(num_threads, sizeof(Node*));
    char* target;
    int i;
    int id;
    Node *cur;
    #pragma omp parallel for shared(hash_table, find_node) private(i, target, id, cur)
    for (i=0; i < hash_table->size; i++) {
        id = omp_get_thread_num();
        cur = hash_table->nodes[i];

        if (cur != NULL) {
            while (cur != NULL) {
                target = (char *)calloc(strlen(cur->key), sizeof(char));
                reverse_word(target, cur->key);

                if (ht_search(hash_table, target)) {
                    Node *node = create_node(target);
                    if (find_node[id] == NULL) {
                        find_node[id] = node;
                    } else {
                        Node *tmp = NULL;
                        
                        tmp = find_node[id];
                        node->next = tmp;
                        find_node[id] = node;
                    }
                }
                free(target);
                cur = cur->next;
            }
        } else {
            continue;
        }
    }
    search_time = omp_get_wtime();

    output = fopen(output_file, "w");
    if (output == NULL) {
        printf("Error opening output_file");
    }

    for(i=0; i < num_threads; i++) {
        Node *tmp = NULL;
        while(find_node[i] != NULL) {
            tmp = find_node[i];
            find_node[i] = tmp->next;
            fprintf(output, "%s\n", tmp->key);
            free(tmp);
        }
    }
    free_htable(hash_table);
    output_time = omp_get_wtime();


    printf("Totaltime = %.6f\n", output_time-start);
    printf("Input I/O time = %.6f\n", input_time-start);
    printf("Search time = %.6f\n", search_time - input_time);
    printf("Output I/O time = %.6f\n", output_time-search_time);

    return 0;
}

unsigned long hash_function (const char *s_) 
{
  const unsigned char *s = (const unsigned char *) s_;
  unsigned hash;

  hash = FNV_32_BASIS;
  while (*s != '\0')
    hash = (hash * FNV_32_PRIME) ^ *s++;

  return hash % CAPACITY;
}

Node* create_node(char *key) {
    Node *node = (Node*) malloc(sizeof(Node));
    node->key = (char*) calloc (strlen(key), sizeof(char));
    node->next = NULL;

    strcpy(node->key, key);
    
    return node;
}

Hashtable* create_htable(int size) {
    Hashtable* htable = (Hashtable*) malloc (sizeof(Hashtable));
    htable->size = size;
    htable->count = 0;
    htable->nodes = (Node**) calloc(htable->size, sizeof(Node*));

    return htable;
}

void free_node(Node* node) {
    free(node->key);
    free(node);
}

void free_htable(Hashtable *htable) {
    Node *tmp = NULL;
    int i;

    for(i=0; i < htable->size; i++) {
        while(htable->nodes[i] != NULL) {
            tmp = htable->nodes[i];
            htable->nodes[i] = htable->nodes[i]->next;
            free_node(tmp);
        }
    }

    free(htable->nodes);
    free(htable);
}

Node* ht_insert(Hashtable* htable, char* key) {
    unsigned long index = hash_function(key);

    Node* node = create_node(key);
    node->next = htable->nodes[index];
    htable->nodes[index] = node;
    htable->count++;

    return node;
}

int ht_search(Hashtable* htable, char* key) {
    unsigned long index = hash_function(key);
    Node* cur_node = htable->nodes[index];

    if (cur_node != NULL) {
        while (cur_node != NULL) {
            if (strcmp(key, cur_node->key) == 0) {
                return TRUE;
            }
            cur_node = cur_node->next;
        }
    } else {
        return FALSE;
    }
    return FALSE;
}

void reverse_word(char *r_word, char *word) {
    // int i;
    // int len = strlen(word);

    // for (i= 0; i < len/2; i++) {
    //     r_word[i] = word[len - i - 1];
    //     r_word[len -i - 1] = word[i];
    // }

    int i, len;
    len = strlen(word);

    for (i=0; i<len; i++) {
        r_word[i] = word[len-i-1];
    }
    r_word[len] = '\0';
}

