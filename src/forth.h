#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef MAX_WORD_DEFINITION_SIZE
// maximum length of user word definitions
#define MAX_WORD_DEFINITION_SIZE 8192
#endif

#ifndef MAX_TOKENS
// maximum number of tokens that can be tokenized at once
#define MAX_TOKENS 8192
#endif

#ifndef FORTH_ERROR_FUNCTION
// function used for interpreter error logging
// this must support printf style vararg formatting
#define FORTH_ERROR_FUNCTION printf
#endif

#ifndef FORTH_OUTPUT_FUNCTION
// function used for interpreter output
// this must support printf style vararg formatting
#define FORTH_OUTPUT_FUNCTION printf
#endif

#define ALPHABET_SIZE 128

enum FORTH_TYPE {
    FORTH_I64,
    FORTH_F64,
    FORTH_REF,
};

typedef struct {
    enum FORTH_TYPE tag;
    union {
        int64_t int64;
        double float64;
        size_t ref;
    };
} forth_type_t;

typedef struct {
    forth_type_t *data;
    int64_t size;
    int64_t top;
} forth_stack_t;

typedef struct {
    forth_stack_t data_stack;
    forth_stack_t control_stack;

    forth_type_t *heap;
    size_t next_address;

    struct trie_node_s *root;
} forth_t;

typedef void (*forth_builtin_ptr)(forth_t *, size_t *, char **, size_t);
typedef void (*forth_ffi_fn_ptr)(forth_t *);

enum TRIE_NODE_TYPE {
    TRIE_NONE,
    TRIE_USERWORD,
    TRIE_BUILTIN,
    TRIE_FFI_FN,
    TRIE_VARIABLE,
};

typedef struct trie_node_s {
    struct trie_node_s *children[ALPHABET_SIZE];
    enum TRIE_NODE_TYPE node_type;
    union {
        char *userword_def;
        forth_builtin_ptr builtin_fn;
        forth_ffi_fn_ptr ffi_fn;
        forth_type_t var;
    };
    size_t userword_def_len;
} trie_node_t;

forth_stack_t stack_init(size_t size);
void stack_resize(forth_stack_t *stack, size_t size);
void stack_destroy(forth_stack_t *stack);
forth_type_t stack_pop(forth_stack_t *stack);
forth_type_t stack_peek(forth_stack_t *stack);
forth_type_t stack_peek_idx(forth_stack_t *stack, int64_t idx);
void stack_push(forth_stack_t *stack, forth_type_t val);

forth_t forth_init(size_t stack_size, size_t heap_size);
void forth_destroy(forth_t *forth);
void forth_define_word(forth_t *forth, const char *name,
                       const char *definition);
const char *forth_lookup_word(forth_t *forth, const char *name);
void forth_import_file(forth_t *forth, const char *filename);
void forth_eval(forth_t *forth, const char *code);
void forth_add_ffi_function(forth_t *forth, const char *name,
                            void (*ffi_fn)(forth_t *));
void forth_define_variable(forth_t *forth, const char *name, forth_type_t *val);

static inline forth_type_t forth_i64(int64_t n) {
    forth_type_t val;
    val.tag = FORTH_I64;
    val.int64 = n;

    return val;
}

static inline forth_type_t forth_f64(double n) {
    forth_type_t val;
    val.tag = FORTH_F64;
    val.float64 = n;

    return val;
}

static inline forth_type_t forth_ref(size_t n) {
    forth_type_t val;
    val.tag = FORTH_REF;
    val.ref = n;

    return val;
}

static inline int strequal(const char *str1, const char *str2) {
    return (strcmp(str1, str2) == 0);
}