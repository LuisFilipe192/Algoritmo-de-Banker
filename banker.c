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

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    return 0;
}