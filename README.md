
# Overview

This library provides an assortment of useful things, mostly of 'I've 
implememted this a million times' variety - lists, growable arrays and such.
Most modules provide containers of some description, but some do not. 
Link it with `-lmiscellany` and include the headers with `<misc/MODULE.h>`

Note that the library provides no `typedef`s to its structs. 

# Modules

In the `docs` directory there are markdown files with detailed descriptions of
each module. `test` directory contains some - very contrived - examples of 
usage of each.

## Arrays `<misc/array.h>`

Growable arrays. Functions to create an array from data pointers, preallocate
memory and append or prepend values/other arrays are provided. Array views that
do allow access to the data of an existing array are also provided, even if in
a rather barebone way.

## Binary trees `<misc/btree.h>`

Binary search trees. Basic operations - insert, lookup, delete, traverse - are
provided. Advanced functionality like rebalancing and reordering is planned.

## Exceptions `<misc/except.h>`

Exceptions. Can be used not as freely as exceptions in other languages, most
notably you have to declare your local variables `volatile` if you intend to
use them in `EXCEPT` blocks. Also be wary of uncaught exceptions - the default
uncaught exception handler simply calls `exit(EXIT_FAILURE)` after printing the
info about the uncaught exception.

## List `<misc/list.h>`

Doubly-linked lists. Most reasonable operations are implemented, including 
sorting. List slices that give a view on a list without copying it are also
provided.

## Maps `<misc/map.h>`

Maps, also known as hashtables. Supports arbitrary data as keys and values.
Basic functionality - insert, remove, look up - is provided. Depending on a 
flag set during creation, maps may be automatically expanded when their load
factor becomes too high.
