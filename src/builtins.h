#pragma once

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

#include "forth.h"
#include "trie.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832
#endif

#define BUILTIN(name)                                                          \
    static void forth_builtin_##name(forth_t *forth, size_t *i, char **words,  \
                                     size_t words_length)

#define REGISTER(name, fn_name)                                                \
    trie_insert_builtin(forth->root, name, forth_builtin_##fn_name)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// : ... ;
BUILTIN(colon) {
    char *new_word = words[++(*i)];
    char def[MAX_WORD_DEFINITION_SIZE];
    size_t idx = 0;

    while (strcmp(words[++(*i)], ";") != 0) {
        idx += sprintf(&def[idx], "%s ", words[*i]);
    }

    trie_insert_userword(forth->root, new_word, def);
}

// dup
BUILTIN(dup) {
    forth_type_t val = stack_peek(&forth->data_stack);
    stack_push(&forth->data_stack, val);
}

// drop
BUILTIN(drop) {
    (void) stack_pop(&forth->data_stack);
}

// swap
BUILTIN(swap) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, b);
    stack_push(&forth->data_stack, a);
}

// over
BUILTIN(over) {
    forth_type_t top = stack_pop(&forth->data_stack);
    forth_type_t second = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, second);
    stack_push(&forth->data_stack, top);
    stack_push(&forth->data_stack, second);
}

// rot
BUILTIN(rot) {
    forth_type_t n3 = stack_pop(&forth->data_stack);
    forth_type_t n2 = stack_pop(&forth->data_stack);
    forth_type_t n1 = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, n2);
    stack_push(&forth->data_stack, n3);
    stack_push(&forth->data_stack, n1);
}

// pick
BUILTIN(pick) {
    int64_t idx = stack_pop(&forth->data_stack).int64;
    forth_type_t nth = stack_peek_idx(&forth->data_stack, idx);
    stack_push(&forth->data_stack, nth);
}

// roll
BUILTIN(roll) {
    int64_t n = stack_pop(&forth->data_stack).int64;

    size_t top = forth->data_stack.top;
    size_t src_idx = top - 1 - n;

    forth_type_t val = forth->data_stack.data[src_idx];

    for (size_t i = src_idx; i < top - 1; i++) {
        forth->data_stack.data[i] = forth->data_stack.data[i + 1];
    }

    forth->data_stack.data[top - 1] = val;
}

// ?dup
BUILTIN(cmp_dup) {
    forth_type_t val = stack_peek(&forth->data_stack);
    if (val.int64 != 0) {
        stack_push(&forth->data_stack, val);
    }
}

// depth
BUILTIN(depth) {
    forth_type_t depth = forth_i64(forth->data_stack.top);
    stack_push(&forth->data_stack, depth);
}

// COMPARISON

// <
BUILTIN(lt) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 < b.int64 ? -1 : 0));
}

// =
BUILTIN(eq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 == b.int64 ? -1 : 0));
}

// >
BUILTIN(gt) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 > b.int64 ? -1 : 0));
}

// >=
BUILTIN(gteq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 >= b.int64 ? -1 : 0));
}

// <=
BUILTIN(lteq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 <= b.int64 ? -1 : 0));
}

// 0<
BUILTIN(ltz) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 < 0 ? -1 : 0));
}

// 0=
BUILTIN(eqz) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 == 0 ? -1 : 0));
}

// 0>
BUILTIN(gtz) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 > 0 ? -1 : 0));
}

// not
BUILTIN(not) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 == 0 ? -1 : 0));
}

// ARITHMETIC AND LOGICAL

// +
BUILTIN(add) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);

    if (a.tag == FORTH_I64) {
        stack_push(&forth->data_stack, forth_i64(a.int64 + b.int64));
    } else if (a.tag == FORTH_REF) {
        stack_push(&forth->data_stack, forth_ref(a.ref + b.ref));
    }
}

// -
BUILTIN(sub) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 - b.int64));
}

// 1+
BUILTIN(add1) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 + 1));
}

// 1-
BUILTIN(sub1) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 - 1));
}

// 2+
BUILTIN(add2) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 + 2));
}

// 2-
BUILTIN(sub2) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(val.int64 - 2));
}

// *
BUILTIN(mul) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 * b.int64));
}

// /
BUILTIN(div) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 / b.int64));
}

// mod
BUILTIN(mod) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 % b.int64));
}

// /mod
BUILTIN(divmod) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 % b.int64));
    stack_push(&forth->data_stack, forth_i64(a.int64 / b.int64));
}

// max
BUILTIN(max) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    if (a.int64 > b.int64) {
        stack_push(&forth->data_stack, a);
    } else {
        stack_push(&forth->data_stack, b);
    }
}

// min
BUILTIN(min) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    if (a.int64 < b.int64) {
        stack_push(&forth->data_stack, a);
    } else {
        stack_push(&forth->data_stack, b);
    }
}

// abs
BUILTIN(abs) {
    forth_type_t val = stack_pop(&forth->data_stack);
    if (val.int64 > 0) {
        stack_push(&forth->data_stack, val);
    } else {
        stack_push(&forth->data_stack, forth_i64(-val.int64));
    }
}

// negate
BUILTIN(negate) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(-val.int64));
}

// and
BUILTIN(and) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 & b.int64));
}

// or
BUILTIN(or) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 | b.int64));
}

// xor
BUILTIN(xor) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 ^ b.int64));
}

// lshift
BUILTIN(lshift) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 << b.int64));
}

// rshift
BUILTIN(rshift) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.int64 >> b.int64));
}

// MEMORY

// @
BUILTIN(load) {
    forth_type_t addr = stack_pop(&forth->data_stack);
    if (addr.tag != FORTH_REF) {
        FORTH_ERROR_FUNCTION("Error: Loading from non-reference type\n");
    }
    stack_push(&forth->data_stack, *(forth_type_t *) addr.ref);
}

// !
BUILTIN(store) {
    forth_type_t addr = stack_pop(&forth->data_stack);
    if (addr.tag != FORTH_REF) {
        FORTH_ERROR_FUNCTION("Error: Storing to non-reference type\n");
    }
    forth_type_t val = stack_pop(&forth->data_stack);
    *(forth_type_t *) addr.ref = val;
}

// ?
BUILTIN(load_print) {
    forth_type_t addr = stack_pop(&forth->data_stack);
    if (addr.tag != FORTH_REF) {
        FORTH_ERROR_FUNCTION("Error: Storing to non-reference type\n");
    }
    forth_type_t val = forth->heap[addr.ref];
    switch (val.tag) {
    case FORTH_I64:
        FORTH_OUTPUT_FUNCTION("%lld ", (long long) val.int64);
        break;
    case FORTH_F64:
        FORTH_OUTPUT_FUNCTION("%f ", val.float64);
        break;
    case FORTH_REF:
        FORTH_OUTPUT_FUNCTION("%zu ", val.ref);
        break;
    default:
        FORTH_OUTPUT_FUNCTION("%lld ", (long long) val.int64);
        break;
    }
}

// CONTROL STRUCTURES

// do
BUILTIN(do) {
    // stack: ... start limit
    forth_type_t index = stack_pop(&forth->data_stack);
    forth_type_t limit = stack_pop(&forth->data_stack);

    // push loop control values in reverse: limit, index, return address
    stack_push(&forth->control_stack, limit);
    stack_push(&forth->control_stack, index);
    stack_push(&forth->control_stack, forth_ref(*i));
}

// loop
BUILTIN(loop) {
    forth_type_t loop_start = stack_pop(&forth->control_stack);
    forth_type_t index = stack_pop(&forth->control_stack);
    forth_type_t limit = stack_pop(&forth->control_stack);

    index.int64 += 1;

    if (index.int64 < limit.int64) {
        stack_push(&forth->control_stack, limit);
        stack_push(&forth->control_stack, index);
        stack_push(&forth->control_stack, loop_start);
        *i = loop_start.ref; // jump back
    }
}

// +loop
BUILTIN(add_loop) {
    int64_t inc = stack_pop(&forth->data_stack).int64;

    // get control stack top
    size_t top = forth->control_stack.top;
    if (top < 3) {
        FORTH_ERROR_FUNCTION("+loop error: control stack underflow\n");
        return;
    }

    forth_type_t addr_val = forth->control_stack.data[top - 1];
    forth_type_t index_val = forth->control_stack.data[top - 2];
    forth_type_t limit_val = forth->control_stack.data[top - 3];

    int64_t new_index = index_val.int64 + inc;
    int64_t limit = limit_val.int64;
    size_t start = addr_val.ref;

    // pop old values
    forth->control_stack.top -= 3;

    if ((inc > 0 && new_index < limit) || (inc < 0 && new_index > limit)) {
        // push updated frame
        stack_push(&forth->control_stack, forth_i64(limit));
        stack_push(&forth->control_stack, forth_i64(new_index));
        stack_push(&forth->control_stack, forth_ref(start));
        *i = start; // jump back to start of loop
    }
}

// leave
BUILTIN(leave) {
    // skip until "loop" or "+loop"
    while (++(*i) < words_length) {
        if (strequal(words[*i], "loop") || strequal(words[*i], "+loop")) {
            break;
        }
    }

    // pop loop frame from control stack
    (void) stack_pop(&forth->control_stack); // index
    (void) stack_pop(&forth->control_stack); // limit
    (void) stack_pop(&forth->control_stack); // start idx
}

// i
BUILTIN(i) {
    size_t top = forth->control_stack.top;
    if (top >= 2) {
        forth_type_t index = forth->control_stack.data[top - 2];
        stack_push(&forth->data_stack, index);
    } else {
        FORTH_ERROR_FUNCTION("Error: control stack underflow in 'i'\n");
    }
}

// j
BUILTIN(j) {
    size_t top = forth->control_stack.top;
    if (top >= 5) {
        forth_type_t index = forth->control_stack.data[top - 5];
        stack_push(&forth->data_stack, index);
    } else {
        FORTH_ERROR_FUNCTION("Error: control stack underflow in 'j'\n");
    }
}

// if
BUILTIN(if) {
    int64_t condition = stack_pop(&forth->data_stack).int64;
    // if false, go to the next "else" or "then", whichever is found
    // first
    if (condition == 0) {
        int64_t depth = 1;
        while (++(*i) < words_length) {
            if (strequal(words[*i], "then")) {
                depth--;
                if (depth == 0) {
                    break;
                }
            } else if (strequal(words[*i], "else") && depth == 1) {
                break;
            }
        }
    } else {
        // true, do nothing
    }
}

// else
BUILTIN(else) {
    // skip to then
    int depth = 1;
    while (++(*i) < words_length) {
        if (strcmp(words[*i], "if") == 0) {
            depth++;
        } else if (strcmp(words[*i], "then") == 0) {
            depth--;
            if (depth == 0) {
                break;
            }
        }
    }
}

// then
BUILTIN(then) {
    // no op
}

// begin
BUILTIN(begin) {
    stack_push(&forth->control_stack, forth_ref((*i) - 1));
}

// again
BUILTIN(again) {
    *i = stack_pop(&forth->control_stack).ref;
}

// until
BUILTIN(until) {
    int64_t flag = stack_pop(&forth->data_stack).int64;
    if (flag == 0) {
        *i = stack_pop(&forth->control_stack).ref;
    } else {
        (void) stack_pop(&forth->control_stack);
    }
}

// cr
BUILTIN(cr) {
    FORTH_OUTPUT_FUNCTION("\n");
}

// emit
BUILTIN(emit) {
    forth_type_t val = stack_pop(&forth->data_stack);
    FORTH_OUTPUT_FUNCTION("%c", (char) val.int64);
}

// space
BUILTIN(space) {
    FORTH_OUTPUT_FUNCTION(" ");
}

// spaces
BUILTIN(spaces) {
    forth_type_t val = stack_pop(&forth->data_stack);
    for (int64_t idx = 0; idx < val.int64; idx++) {
        FORTH_OUTPUT_FUNCTION(" ");
    }
}

// page
BUILTIN(page) {
    // ANSI clear followed by ANSI cursor home
    FORTH_OUTPUT_FUNCTION("\033[2J\033[H");
}

// dump
BUILTIN(dump) {
    FORTH_OUTPUT_FUNCTION("Stack dump: \n");
    for (int64_t idx = forth->data_stack.top - 1; idx >= 0; idx--) {
        forth_type_t val = forth->data_stack.data[idx];
        switch (val.tag) {
        case FORTH_I64:
            FORTH_OUTPUT_FUNCTION("%lld (I64)\n", (long long) val.int64);
            break;
        case FORTH_F64:
            FORTH_OUTPUT_FUNCTION("%f (F64)\n", val.float64);
            break;
        case FORTH_REF:
            FORTH_OUTPUT_FUNCTION("%zu (REF)\n", val.ref);
            break;
        default:
            FORTH_OUTPUT_FUNCTION("%zu (BAD TAG (%d))\n", val.ref, val.tag);
            break;
        }
    }
}

// .
BUILTIN(period) {
    forth_type_t val = stack_pop(&forth->data_stack);
    if (val.tag == FORTH_I64) {
        FORTH_OUTPUT_FUNCTION("%lld ", (long long) val.int64);
    } else if (val.tag == FORTH_F64) {
        FORTH_OUTPUT_FUNCTION("%f ", val.float64);
    } else if (val.tag == FORTH_REF) {
        FORTH_OUTPUT_FUNCTION("%zu ", val.ref);
    }
}

// variable
BUILTIN(variable) {
    char *variable_name = words[++(*i)];
    forth_type_t *addr = &forth->heap[forth->next_address++];
    addr->tag = FORTH_I64;
    addr->int64 = 0;
    forth_define_variable(forth, variable_name, addr);
}

// include
BUILTIN(include) {
    char *filename = words[++(*i)];
    forth_import_file(forth, filename);
}

// ref
BUILTIN(ref) {
    char *ref_str = words[++(*i)];
    char *endptr;

    errno = 0;
    size_t ref = (size_t) strtoumax(ref_str, &endptr, 10);
    if (errno != 0) {
        FORTH_ERROR_FUNCTION("Failed to convert word '%s' to reference\n",
                             ref_str);
    }
    forth_type_t val = forth_ref(ref);
    stack_push(&forth->data_stack, val);
}

// FLOATING POINT

// d>f
BUILTIN(d_to_f) {
    int64_t d = stack_pop(&forth->data_stack).int64;
    forth_type_t f = forth_f64((double) d);
    stack_push(&forth->data_stack, f);
}

// f>d
BUILTIN(f_to_d) {
    double f = stack_pop(&forth->data_stack).int64;
    forth_type_t d = forth_i64((int64_t) f);
    stack_push(&forth->data_stack, d);
}

// f+
BUILTIN(fadd) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(a.float64 + b.float64));
}

// f-
BUILTIN(fsub) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(a.float64 - b.float64));
}

// f*
BUILTIN(fmul) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(a.float64 * b.float64));
}

// f/
BUILTIN(fdiv) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(a.float64 / b.float64));
}

// fnegate
BUILTIN(fnegate) {
    forth_type_t val = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(-val.float64));
}

// fabs
BUILTIN(fabs) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(fabs(x.float64)));
}

// fmax
BUILTIN(fmax) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(fmax(a.float64, b.float64)));
}

// fmin
BUILTIN(fmin) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(fmin(a.float64, b.float64)));
}

// floor
BUILTIN(floor) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(floor(x.float64)));
}

// fround
BUILTIN(fround) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(round(x.float64)));
}

// f**
BUILTIN(fpow) {
    forth_type_t exp = stack_pop(&forth->data_stack);
    forth_type_t base = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(pow(base.float64, exp.float64)));
}

// 1/f
BUILTIN(one_div_f) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(1.0 / x.float64));
}

// f2/
BUILTIN(f_div_two) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(x.float64 / 2.0));
}

// fsin
BUILTIN(fsin) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(sin(x.float64)));
}

// fcos
BUILTIN(fcos) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(cos(x.float64)));
}

// fsincos
BUILTIN(fsincos) {
    forth_type_t x = stack_pop(&forth->data_stack);
    double s, c;
    s = sin(x.float64);
    c = cos(x.float64);
    stack_push(&forth->data_stack, forth_f64(c)); // push cos first
    stack_push(&forth->data_stack, forth_f64(s)); // then sin
}

// ftan
BUILTIN(ftan) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(tan(x.float64)));
}

// fasin
BUILTIN(fasin) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(asin(x.float64)));
}

// facos
BUILTIN(facos) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(acos(x.float64)));
}

// fatan
BUILTIN(fatan) {
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(atan(x.float64)));
}

// fatan2
BUILTIN(fatan2) {
    forth_type_t y = stack_pop(&forth->data_stack);
    forth_type_t x = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_f64(atan2(y.float64, x.float64)));
}

// pi
BUILTIN(pi) {
    stack_push(&forth->data_stack, forth_f64(M_PI));
}

// f~rel
BUILTIN(fapprox_rel) {
    forth_type_t rel = stack_pop(&forth->data_stack);
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    double diff = fabs(a.float64 - b.float64);
    stack_push(
        &forth->data_stack,
        forth_i64(diff <= rel.float64 * fmax(fabs(a.float64), fabs(b.float64))
                      ? -1
                      : 0));
}

// f~abs
BUILTIN(fapprox_abs) {
    forth_type_t abs_tol = stack_pop(&forth->data_stack);
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(
        &forth->data_stack,
        forth_i64(fabs(a.float64 - b.float64) <= abs_tol.float64 ? -1 : 0));
}

// f~
BUILTIN(fapprox) {
    forth_type_t abs_tol = stack_pop(&forth->data_stack);
    forth_type_t rel_tol = stack_pop(&forth->data_stack);
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    double diff = fabs(a.float64 - b.float64);
    double max_ab = fmax(fabs(a.float64), fabs(b.float64));
    int result =
        (diff <= abs_tol.float64) || (diff <= rel_tol.float64 * max_ab);
    stack_push(&forth->data_stack, forth_i64(result ? -1 : 0));
}

// f=
BUILTIN(feq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 == b.float64 ? -1 : 0));
}

// f<>
BUILTIN(fneq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 != b.float64 ? -1 : 0));
}

// f<
BUILTIN(flt) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 < b.float64 ? -1 : 0));
}

// f<=
BUILTIN(flteq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 <= b.float64 ? -1 : 0));
}

// f>
BUILTIN(fgt) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 > b.float64 ? -1 : 0));
}

// f>=
BUILTIN(fgteq) {
    forth_type_t b = stack_pop(&forth->data_stack);
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 >= b.float64 ? -1 : 0));
}

// f0<
BUILTIN(fltz) {
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 < 0 ? -1 : 0));
}

// f0<=
BUILTIN(flteqz) {
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 <= 0 ? -1 : 0));
}

// f0<>
BUILTIN(fnz) {
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 != 0 ? -1 : 0));
}

// f0=
BUILTIN(feqz) {
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 == 0 ? -1 : 0));
}

// f0>
BUILTIN(fgtz) {
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 > 0 ? -1 : 0));
}

// f0>=
BUILTIN(fgteqz) {
    forth_type_t a = stack_pop(&forth->data_stack);
    stack_push(&forth->data_stack, forth_i64(a.float64 >= 0 ? -1 : 0));
}

// bye
BUILTIN(bye) {
    exit(0);
}

// throw
BUILTIN(throw) {
    int64_t err = stack_pop(&forth->data_stack).int64;
    if (err != 0) {
        exit(err);
    }
}

#pragma GCC diagnostic pop

void forth_register_all_builtins(forth_t *forth) {
    REGISTER(":", colon);
    REGISTER("dup", dup);
    REGISTER("drop", drop);
    REGISTER("swap", swap);
    REGISTER("over", over);
    REGISTER("rot", rot);
    REGISTER("pick", pick);
    REGISTER("roll", roll);
    REGISTER("?dup", cmp_dup);
    REGISTER("depth", depth);
    REGISTER("<", lt);
    REGISTER("=", eq);
    REGISTER(">", gt);
    REGISTER(">=", gteq);
    REGISTER("<=", lteq);
    REGISTER("0<", ltz);
    REGISTER("0=", eqz);
    REGISTER("0>", gtz);
    REGISTER("not", not);
    REGISTER("+", add);
    REGISTER("-", sub);
    REGISTER("1+", add1);
    REGISTER("1-", sub1);
    REGISTER("2+", add2);
    REGISTER("2-", sub2);
    REGISTER("*", mul);
    REGISTER("/", div);
    REGISTER("mod", mod);
    REGISTER("/mod", divmod);
    REGISTER("max", max);
    REGISTER("min", min);
    REGISTER("abs", abs);
    REGISTER("negate", negate);
    REGISTER("and", and);
    REGISTER("or", or);
    REGISTER("xor", xor);
    REGISTER("lshift", lshift);
    REGISTER("rshift", rshift);
    REGISTER("@", load);
    REGISTER("!", store);
    REGISTER("?", load_print);
    REGISTER("do", do);
    REGISTER("loop", loop);
    REGISTER("+loop", add_loop);
    REGISTER("leave", leave);
    REGISTER("i", i);
    REGISTER("j", j);
    REGISTER("if", if);
    REGISTER("else", else);
    REGISTER("then", then);
    REGISTER("begin", begin);
    REGISTER("again", again);
    REGISTER("until", until);
    REGISTER("cr", cr);
    REGISTER("emit", emit);
    REGISTER("space", space);
    REGISTER("spaces", spaces);
    REGISTER("page", page);
    REGISTER("dump", dump);
    REGISTER(".", period);
    REGISTER("variable", variable);
    REGISTER("include", include);
    REGISTER("ref", ref);
    REGISTER("d>f", d_to_f);
    REGISTER("f>d", f_to_d);
    REGISTER("f+", fadd);
    REGISTER("f-", fsub);
    REGISTER("f*", fmul);
    REGISTER("f/", fdiv);
    REGISTER("fnegate", fnegate);
    REGISTER("fabs", fabs);
    REGISTER("fmax", fmax);
    REGISTER("fmin", fmin);
    REGISTER("floor", floor);
    REGISTER("fround", fround);
    REGISTER("f**", fpow);
    REGISTER("1/f", one_div_f);
    REGISTER("f2/", f_div_two);
    REGISTER("fsin", fsin);
    REGISTER("fcos", fcos);
    REGISTER("fsincos", fsincos);
    REGISTER("ftan", ftan);
    REGISTER("fasin", fasin);
    REGISTER("ftan", ftan);
    REGISTER("fasin", fasin);
    REGISTER("facos", facos);
    REGISTER("fatan", fatan);
    REGISTER("fatan2", fatan2);
    REGISTER("pi", pi);
    REGISTER("f~rel", fapprox_rel);
    REGISTER("f~abs", fapprox_abs);
    REGISTER("f~", fapprox);
    REGISTER("f=", feq);
    REGISTER("f<>", fneq);
    REGISTER("f<", flt);
    REGISTER("f<=", flteq);
    REGISTER("f>", fgt);
    REGISTER("f>=", fgteq);
    REGISTER("f0<", fltz);
    REGISTER("f0<=", flteqz);
    REGISTER("f0<>", fnz);
    REGISTER("f0=", feqz);
    REGISTER("f0>", fgtz);
    REGISTER("f0>=", fgteqz);
    REGISTER("bye", bye);
    REGISTER("throw", throw);
}

#undef REGISTER
#undef BUILTIN