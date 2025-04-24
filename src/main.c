#include <stdio.h>

#include <readline/history.h>
#include <readline/readline.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "forth.h"

// ffi function example
void ffi_rand(forth_t *forth) {
    uint64_t n = rand();
    stack_push(&forth->data_stack, forth_i64(n));
}

int main(int argc, char *argv[]) {
    forth_t forth =
        forth_init(sizeof(forth_type_t) * 4096, sizeof(forth_type_t) * 4096);

    // regiserting ffi_rand as a forth word
    forth_add_ffi_function(&forth, "rand", ffi_rand);

    // handle ^C and ^D in readline
    rl_getc_function = getc;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            forth_import_file(&forth, argv[i]);
        }
    }

    while (1) {
        char *line = readline("meili> ");
        if (line == NULL) {
            break;
        }

        add_history(line);
        forth_eval(&forth, line);
        printf(" ok\n");
        free(line);
    }

    rl_clear_history();
    forth_destroy(&forth);
    return 0;
}