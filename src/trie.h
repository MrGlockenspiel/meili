#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "forth.h"

static trie_node_t *trie_create_blank_node(void) {
    trie_node_t *node = (trie_node_t *) malloc(sizeof(trie_node_t));

    node->node_type = TRIE_NONE;
    node->userword_def = NULL;

    for (size_t i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

static void trie_insert_ffi_function(trie_node_t *root, const char *key,
                                     forth_ffi_fn_ptr ffi_fn) {
    trie_node_t *current = root;

    for (size_t i = 0; i < strlen(key); i++) {
        size_t idx = key[i];
        if (current->children[idx] == NULL) {
            current->children[idx] = trie_create_blank_node();
        }
        current = current->children[idx];
    }

    current->node_type = TRIE_FFI_FN;
    current->ffi_fn = ffi_fn;
}

static void trie_insert_builtin(trie_node_t *root, const char *key,
                                forth_builtin_ptr builtin_fn) {
    trie_node_t *current = root;

    for (size_t i = 0; i < strlen(key); i++) {
        size_t idx = key[i];
        if (current->children[idx] == NULL) {
            current->children[idx] = trie_create_blank_node();
        }
        current = current->children[idx];
    }

    current->node_type = TRIE_BUILTIN;
    current->builtin_fn = builtin_fn;
}

static void trie_insert_userword(trie_node_t *root, const char *key,
                                 const char *def) {
    trie_node_t *current = root;

    for (size_t i = 0; i < strlen(key); i++) {
        size_t idx = key[i];
        if (current->children[idx] == NULL) {
            current->children[idx] = trie_create_blank_node();
        }
        current = current->children[idx];
    }

    current->node_type = TRIE_USERWORD;
    size_t len = strlen(def);
    current->userword_def_len = len;
    current->userword_def = (char *) malloc(len);
    strncpy(current->userword_def, def, len);
    current->userword_def[len - 1] = '\0';
}

static void trie_insert_variable(trie_node_t *root, const char *key,
                                 forth_type_t val) {
    trie_node_t *current = root;

    for (size_t i = 0; i < strlen(key); i++) {
        size_t idx = key[i];
        if (current->children[idx] == NULL) {
            current->children[idx] = trie_create_blank_node();
        }
        current = current->children[idx];
    }

    current->node_type = TRIE_VARIABLE;
    current->var = val;
}

static trie_node_t *trie_search(trie_node_t *root, const char *key) {
    trie_node_t *current = root;
    for (size_t i = 0; i < strlen(key); i++) {
        size_t idx = key[i];
        if (current->children[idx] == NULL) {
            return NULL;
        }
        current = current->children[idx];
    }
    if (current != NULL && current->node_type != TRIE_NONE) {
        return current;
    }
    return NULL;
}

static void trie_destroy(trie_node_t *node) {
    if (node == NULL) {
        return;
    }

    for (size_t i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            trie_destroy(node->children[i]);
            node->children[i] = NULL;
        }
    }

    if (node->node_type == TRIE_USERWORD && node->userword_def != NULL) {
        free(node->userword_def);
        node->userword_def = NULL;
    }

    free(node);
}