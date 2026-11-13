#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define LINE_MAX_LEN 1024
#define CMD_RQ   1
#define CMD_RL   2
#define CMD_STAR 3

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
        }
        i++;
    }
    fclose(f);
}

static int parse_command_line(char *line, int *type, int *customer, int *values){
    char *tok = strtok(line," \t");

    if(tok == NULL){
        return 0;
    }

    if(strcmp(tok,"*") == 0){
        *type = CMD_STAR;
        return strtok(NULL," \t") == NULL;
    }

    if (strcmp(tok, "RQ") == 0){
        *type = CMD_RQ;
    }
    else if (strcmp(tok,"RL") == 0){
        *type = CMD_RL;
    }
    else{
        return 0;
    }

    tok = strtok(NULL," \t");

    if (!parse_nonneg_int(tok, customer)){
        return 0;
    }
    if(*customer >= n_customers){
        return 0;
    }
    for(int j = 0; j < n_resources; j++){
        tok = strtok(NULL, " \t");
        if(!parse_nonneg_int(tok, &values[j])){
            return 0;
        }
    }

    return strtok(NULL," \t") == NULL;
}

static void validate_commands(const char *path){

    FILE *f = fopen(path, "r");
    if (f == NULL){
        die("could not open commands.txt");
    }

    int *values = malloc(n_resources * sizeof(int));

    if(values == NULL){
        die("out of memory");
    }

    char line[LINE_MAX_LEN];
    int lineno = 0;
    int type, customer;

    while(fgets(line, sizeof(line), f) != NULL){
        lineno++;
        strip_newline(line);

        if(is_blank(line)){
            continue;
        }

        if(!parse_command_line(line, &type, &customer, values)){
            fprintf(stderr, "Error: commands.txt line %d is invalid "
                    "(expected 'RQ <customer> <%d values>', "
                    "'RL <customer> <%d values>' or '*')\n",
                    lineno, n_resources, n_resources);
            free(values);
            fclose(f);
            exit(EXIT_FAILURE);
        }
    }

    free(values);
    fclose(f);
}

static int is_safe(void){
    int *work = malloc(n_resources*sizeof(int));
    int *finish = calloc(n_customers,sizeof(int));

    if (work == NULL || finish == NULL){
        die("out of memory");
    }
    for(int j = 0; j < n_resources; j++){
        work[j] = available[j];
    }

    int progress = 1;
    
    while(progress){
        progress = 0;
        for(int i=0;i < n_customers;i++){
            if(finish[i]){
                continue;
            }

            int can_finish = 1;
            for(int j = 0;j < n_resources;j++){
                if(need[i][j] > work[j]){
                    can_finish = 0;
                    break;
                }
            }

            if(can_finish){
                for (int j = 0; j < n_resources; j++){
                    work[j] += allocation[i][j];
                }
                finish[i] = 1;
                progress = 1;
            }
        }
    }

    int safe = 1;
    for(int i=0;i < n_customers;i++){
        if (!finish[i]) safe = 0;
    }

    free(work);
    free(finish);
    return safe;
}

static void print_vector(FILE *out, const int *v){
    for(int j = 0; j < n_resources; j++){
        fprintf(out,j == 0 ? "%d" : " %d",v[j]);
    }
}

static void request_resources(FILE *out, int c, const int *req){
    for(int j = 0; j < n_resources; j++){
        if(req[j]> need[c][j]){
            fprintf(out,"The customer %d request ",c);
            print_vector(out,req);
            fprintf(out," was denied because exceed its maximum allocation\n");
            return;
        }
    }

    for(int j=0;j< n_resources;j++){
        if (req[j] > available[j]) {
            fprintf(out,"The resources ");
            print_vector(out, available);
            fprintf(out, " are not enough to customer %d request ", c);
            print_vector(out, req);
            fprintf(out,"\n");
            return;
        }
    }

    for(int j = 0; j < n_resources; j++){
        available[j]-=req[j];
        allocation[c][j]+=req[j];
        need[c][j]-=req[j];
    }

    if(!is_safe()){
        for(int j=0; j < n_resources;j++){
            available[j]+=req[j];
            allocation[c][j]-=req[j];
            need[c][j]+=req[j];
        }

        fprintf(out,"The customer %d request ",c);
        print_vector(out, req);
        fprintf(out," was denied because result in an unsafe state\n");
        return;
    }

    fprintf(out,"Allocate to customer %d the resources ", c);
    print_vector(out,req);
    fprintf(out,"\n");
}

static void release_resources(FILE *out, int c, const int *rel){
    for(int j = 0; j < n_resources; j++){
        if(rel[j] > allocation[c][j]){
            fprintf(out,"The customer %d release ", c);
            print_vector(out,rel);
            fprintf(out, " was denied because exceed its allocation\n");
            return;
        }
    }

    for(int j = 0; j < n_resources; j++){
        allocation[c][j]-= rel[j];
        available[j]+=rel[j];
        need[c][j]+=rel[j];
    }

    fprintf(out, "Release from customer %d the resources ",c);
    print_vector(out,rel);
    fprintf(out,"\n");
}

static void print_state(FILE *out){
    fprintf(out,"MAXIMUM | ALLOCATION | NEED\n");
    for(int i=0;i < n_customers;i++){
        print_vector(out,maximum[i]);
        fprintf(out," | ");
        print_vector(out,allocation[i]);
        fprintf(out," | ");
        print_vector(out,need[i]);
        fprintf(out,"\n");
    }

    fprintf(out,"AVAILABLE ");
    print_vector(out,available);
    fprintf(out,"\n");
}

static void run_commands(const char *in_path, const char *out_path){
    FILE *in = fopen(in_path, "r");

    if(in == NULL){
        die("could not open commands.txt");
    }

    FILE *out = fopen(out_path, "w");
    
    if(out == NULL){
        fclose(in);
        die("could not create result.txt");
    }

    int *values=malloc(n_resources * sizeof(int));
    if(values == NULL){
        die("out of memory");
    }

    char line[LINE_MAX_LEN];
    int type, customer;

    while(fgets(line, sizeof(line), in) != NULL){
        strip_newline(line);

        if(is_blank(line)){
            continue;
        }

        parse_command_line(line, &type, &customer, values);

        if(type == CMD_RQ){
            request_resources(out, customer, values);
        }
        else if(type == CMD_RL){
            release_resources(out, customer, values);
        }
        else{
            print_state(out);
        }
    }

    free(values);
    fclose(out);
    fclose(in);
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    load_customers("customer.txt");
    validate_commands("commands.txt");
    run_commands("commands.txt", "result.txt");
    return 0;
}