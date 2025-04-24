#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "forth.h"
#include "trie.h"

forth_stack_t stack_init(size_t size) {
    forth_stack_t stack;

    stack.data = malloc(size);
    stack.size = size;
    stack.top = 0;

    return stack;
}

void stack_resize(forth_stack_t *stack, size_t size) {
    stack->data = realloc(stack->data, size);
    stack->size = size;
}

void stack_destroy(forth_stack_t *stack) {
    free(stack->data);
    stack->data = NULL;
    stack->size = 0;
    stack->top = 0;
}

forth_type_t stack_pop(forth_stack_t *stack) {
    stack->top--;

    if (stack->top < 0) {
        FORTH_OUTPUT_FUNCTION("%s:%d: Stack underflow:\n", __FILE_NAME__,
                              __LINE__);
    }

    if (stack->top > stack->size) {
        FORTH_OUTPUT_FUNCTION("%s:%d: Stack overflow:\n", __FILE_NAME__,
                              __LINE__);
    }

    return stack->data[stack->top];
}

forth_type_t stack_peek(forth_stack_t *stack) {
    return stack->data[stack->top - 1];
}

forth_type_t stack_peek_idx(forth_stack_t *stack, int64_t idx) {
    return stack->data[stack->top - idx];
}

void stack_push(forth_stack_t *stack, forth_type_t val) {
    stack->data[stack->top] = val;
    stack->top++;

    if (stack->top < 0) {
        FORTH_OUTPUT_FUNCTION(
            "src2/%s:%d: Stack underflow: Top: %zu, Size: %zu\n", __FILE_NAME__,
            __LINE__, stack->top, stack->size);
    }

    if (stack->top > stack->size) {
        FORTH_OUTPUT_FUNCTION(
            "src2/%s:%d: Stack overflow: Top: %zu, Size: %zu\n", __FILE_NAME__,
            __LINE__, stack->top, stack->size);
    }
}

forth_t forth_init(size_t stack_size, size_t heap_size) {
    forth_t forth;
    memset(&forth, 0, sizeof(forth));

    forth.data_stack = stack_init(stack_size);
    forth.control_stack = stack_init(stack_size);

    forth.heap = malloc(sizeof(forth_type_t) * heap_size);
    forth.next_address = 0;

    forth.root = trie_create_blank_node();

    forth_register_all_builtins(&forth);

    return forth;
}

void forth_destroy(forth_t *forth) {
    stack_destroy(&forth->data_stack);
    stack_destroy(&forth->control_stack);

    free(forth->heap);
    forth->heap = NULL;

    trie_destroy(forth->root);
}

void forth_define_word(forth_t *forth, const char *name,
                       const char *definition) {
    trie_insert_userword(forth->root, name, definition);
}

static int parse_integer(const char *word, int64_t *out) {
    char *end;
    errno = 0;

    int64_t val = strtoll(word, &end, 10);

    if (*end == '\0' && errno == 0) {
        *out = val;
        return 1;
    }

    return 0;
}

static int parse_float(const char *word, double *out) {
    char *end;
    errno = 0;

    double val = strtod(word, &end);

    if (*end == '\0' && errno == 0) {
        *out = val;
        return 1;
    }

    return 0;
}

char *remove_comments(const char *input) {
    size_t len = strlen(input);
    char *cleaned = malloc(len + 1);
    if (cleaned == NULL) {
        return NULL;
    }

    int in_parens = 0;
    size_t j = 0;

    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\') {
            // skip to the end of the line
            while (i < len && input[i] != '\n') {
                i++;
            }
        }

        if (input[i] == '(') {
            in_parens++;
            continue;
        } else if (input[i] == ')' && in_parens) {
            in_parens--;
            continue;
        }

        if (!in_parens && input[i] != '#' && input[i] != '\r') {
            cleaned[j++] = input[i];
        }
    }

    cleaned[j] = '\0';
    return cleaned;
}

char **tokenize_words(const char *code, size_t *out_count) {
    char *cleaned = remove_comments(code);
    if (cleaned == NULL) {
        return NULL;
    }

#define MAX_TOKENS 8192

    char **tokens = malloc(sizeof(char *) * MAX_TOKENS);
    int count = 0;

    char *token = strtok(cleaned, " \t\n");
    while (token && count < MAX_TOKENS) {
        tokens[count++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }

#undef MAX_TOKENS

    free(cleaned);
    *out_count = count;
    return tokens;
}

void forth_import_file(forth_t *forth, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        FORTH_ERROR_FUNCTION("Error opening '%s'\n", filename);
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    rewind(fp);

    char *buffer = malloc(len + 1);
    fread(buffer, len, 1, fp);
    buffer[len] = '\0';

    forth_eval(forth, buffer);
    free(buffer);
}

void forth_add_ffi_function(forth_t *forth, const char *name,
                            forth_ffi_fn_ptr fn) {
    trie_insert_ffi_function(forth->root, name, fn);
}

void forth_define_variable(forth_t *forth, const char *name,
                           forth_type_t *val) {
    trie_insert_variable(forth->root, name, forth_ref((size_t) val));
}

forth_type_t *forth_get_variable(forth_t *forth, const char *name) {
    forth_eval(forth, name);
    forth_type_t addr = stack_pop(&forth->data_stack);
    if (addr.tag != FORTH_REF) {
        FORTH_ERROR_FUNCTION("Error: Dereferencing non-reference type\n");
    }

    forth_type_t *ptr = (forth_type_t *) addr.ref;
    return ptr;
}

void forth_eval(forth_t *forth, const char *code) {
    size_t words_length;
    char **words = tokenize_words(code, &words_length);

    for (size_t i = 0; i < words_length; i++) {
        int64_t i64_val;
        double f64_val;

        const char *word = words[i];

        if (parse_integer(word, &i64_val)) {
            stack_push(&forth->data_stack, forth_i64(i64_val));
        } else if (parse_float(word, &f64_val)) {
            stack_push(&forth->data_stack, forth_f64(f64_val));
        } else {
            trie_node_t *node = trie_search(forth->root, word);
            // printf("node for '%s' at %p\n", word, node);

            if (node == NULL) {
                FORTH_ERROR_FUNCTION("Error: word '%s' undefined\n", word);
                return;
            }

            switch (node->node_type) {
            case TRIE_NONE:
                FORTH_ERROR_FUNCTION(
                    "Error: word '%s' defined with no node type\n", word);
                break;
            case TRIE_FFI_FN:
                node->ffi_fn(forth);
                break;
            case TRIE_USERWORD:
                forth_eval(forth, node->userword_def);
                break;
            case TRIE_BUILTIN:
                node->builtin_fn(forth, &i, words, words_length);
                break;
            case TRIE_VARIABLE:
                stack_push(&forth->data_stack, node->var);
                break;
            }
        }
    }

    for (size_t i = 0; i < words_length; i++) {
        free(words[i]);
    }
    free(words);
}