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

static void strip_newline(char *s){
    size_t len = strlen(s);

    while(len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')){
        s[--len] = '\0';
    }
}

static int is_blank(const char *s){
    for(; *s; s++){
        if(!isspace((unsigned char)*s)){
            return 0;
        }
    }
    return 1;
}

static int parse_customer_line(char *line, int *values) {
    int count = 0;
    char *tok = strtok(line, ",");

    while(tok != NULL){
        int v;
        
        if (!parse_nonneg_int(tok, &v)){
            return 0;
        }
        if (count >= n_resources){
            return 0;
        }
        if(values != NULL){
            values[count] = v;
        }
        count++;
        tok = strtok(NULL, ",");
    }

    return count == n_resources;
}

#define LINE_MAX_LEN 1024

static void load_customers(const char *path){
    FILE *f = fopen(path, "r");
    if(f == NULL){
        die("could not open customer.txt");
    }

    char line[LINE_MAX_LEN];
    int lineno = 0;

    while (fgets(line, sizeof(line), f) != NULL){
        lineno++;
        strip_newline(line);

        if(is_blank(line)){
            continue;
        }
        if(!parse_customer_line(line, NULL)){
            fprintf(stderr,"Error: customer.txt line %d is invalid "
                "(expected %d comma-separated non-negative integers)\n",
                lineno, n_resources);
            fclose(f);
            exit(EXIT_FAILURE);
        }

        n_customers++;
    }

    if(n_customers == 0){
        fclose(f);
        die("customer.txt has no customers");
    }

    maximum    = alloc_matrix(n_customers, n_resources);
    allocation = alloc_matrix(n_customers, n_resources);
    need       = alloc_matrix(n_customers, n_resources);

    rewind(f);
    int i = 0;

    while(fgets(line, sizeof(line), f) != NULL){
        strip_newline(line);
        if (is_blank(line)){
            continue;
        }

        parse_customer_line(line, maximum[i]);

        for (int j = 0; j < n_resources; j++){
            need[i][j] = maximum[i][j];
        i++;
        }
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    load_customers("customer.txt");
    return 0;
}