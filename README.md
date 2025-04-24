# meili

An embeddable FORTH interpreter written in C.
The interpreter itself lives in `forth.c`, `forth.h`, `trie.h`, and `builtins.h`, while a simple REPL with example usage is in `main.c`

### Details

This is not intended to be a standards compliant implementation of FORTH, but with some work it probably could be. It has a unified stack for both 64 bit signed integers, 64 bit floating point numbers, and unsigned 64 bit reference values. It also has a simple FFI interface to register C functions as FORTH words (Example in `main.c`). It does NOT support strings, arrays (with `cells` and `allocate`), or smaller datatypes. For a complete list of supported words, see `builtins.h`.

### Building

You'll need a C compiler, `make`, `libreadline`, and `pkg-config` to build the interpreter and REPL. Just run `make`, and it'll build the binary `meili`.

### Examples

I've written a few example programs, available in the `forth/` directory over time to test functionality.