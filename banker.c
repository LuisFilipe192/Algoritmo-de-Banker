#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int n_customers = 0;   
static int n_resources = 0;

static int *available = NULL;
static int **maximum = NULL;
static int **allocation = NULL;
static int **need = NULL;

static void die(const char *msg){
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static int parse_nonneg_int(const char *s, int *out) {
    if(s == NULL || *s == '\0'){
        return 0;
    }

    for(const char *p = s; *p; p++){
        if (!isdigit((unsigned char)*p)){
            return 0;    
        }
    }

    long v = strtol(s, NULL, 10);

    if (v > 1000000){
        return 0;
    }

    *out = (int)v;
    return 1;
}

static int **alloc_matrix(int rows, int cols) {
    int **m = malloc(rows * sizeof(int*));
    if(m == NULL){
        die("out of memory");
    }

    for(int i = 0; i < rows; i++){
        m[i] = calloc(cols, sizeof(int));
        if (m[i] == NULL) die("out of memory");
    }
    return m;
}

static void parse_args(int argc, char *argv[]) {
    if(argc < 2){
        die("usage: ./banker <instances of resource 0> <instances of resource 1> ...");
    }

    n_resources = argc - 1;
    available = malloc(n_resources * sizeof(int));
    
    if (available == NULL){
        die("out of memory");
    }
    for (int j = 0; j < n_resources; j++){
        if (!parse_nonneg_int(argv[j + 1], &available[j])){
            die("arguments must be non-negative integers");
        }
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    return 0;
}