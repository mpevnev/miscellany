
# Exceptions module `<misc/except.h>`

This module provides exceptions mechanism much like in C++ or Python (more 
Python, really). 

## Initialization

To set up the exception handling system, call `exc_init` in *every thread* that
uses exceptions. 

You may optionally call `exc_set_uncaught_handler` to set a function that will
handle uncaught exceptions in a given thread. For details, see **Uncaught 
exceptions** section below.

## Exceptions

Exceptions are represented by `struct except` objects. They carry the following
information: namespace they belong to, their type within the namespace, and
some additional data which is assigned to them at the throwing site.

Exceptions built into the library use namespace = 0, so consider it reserved
and don't use it in your applications.

Currently the following built-in exceptions types are available (in parentheses
after is the name of the variable holding such an exception):
- `EXC_OOM` (`exc_oom_exception`)

Exceptions are created with `exc_create` function:
```
struct except *
exc_create(int namespace, int type, void *data, void (*data_cleaner)(void *))
```
This will create an exception with namespace `namespace`, type `type` and 
holding an object pointed to by `data`. If `data_cleaner` is not NULL, it will
be called on exception's data when it's destroyed.

It is up to the user of the library to dispose of an exception after it has
been caught. Just don't do this with built-in exceptions, please.
Exceptions are destroyed with 
```
void
exc_destroy(struct except *exc);
```

Data held by an exception is available via 
```
void *
exc_data(struct except *exc)
```

## Throwing exceptions

There are three functions to throw an exception:
- `exc_throw`,
- `exc_throw_new`,
- `exc_rethrow`.

### `exc_throw`

```
_Noreturn void
exc_throw(struct except *exc)
```

Throw exception `exc`. This is what should be used to throw built-in exceptions.

### `exc_throw_new`

```
_Noreturn void
exc_throw_new(int namespace, int type, void *data, void (*data_cleaner)(void *))
```

Create a new exception (the meaning of the parameters is the same as for 
`exc_create`), then throw it. If there's not enough memory to create a new
exception, a built-in OOM exception is thrown instead.

### `exc_rethrow`

```
_Noreturn void
exc_rethrow(struct except *exc, void *new_data, void (*new_cleaner)(void *))
```

Clean `exc`'s data, assign `new_data` and `new_cleaner` to it, then throw it
again.

## Catching exceptions

The framework for handling exceptions looks like this:

```
TRY
{
	/* Some code */
}
START_EXCEPT;
EXCEPT(namespace, type, variable_name)
{
	/* Code to handle an exception with namespace 'namespace' and type 'type'.
	   The exception will be available via a variable 'variable_name'. */
	...
	/* Don't forget to destroy the exception if you don't need it anymore! */
	exc_destroy(variable_name);
}
/* More EXCEPT blocks, if necessary. */
END_TRY;
```

**Note:** the whole shebang uses `setjmp` under the hood, so if you want to use
any variable from `TRY` block in an `EXCEPT` block, make it `volatile`.

## Uncaught exceptions

If an exception was thrown, but not caught, the uncaught exception handler for
the current thread is called. The default one just prints uncaught exception's
namespace and type and exits with `exit(EXIT_FAILURE)`. You can install a 
custom handler with
```
void
exc_set_uncaught_handler(exc_unc_handler_t handler)
```
where `exc_unc_handler_t` is `typedef void (*exc_unc_handler_t)(struct except *)`.

**Note:** a custom handler should be `noreturn`. If it's not,
`exit(EXIT_FAILURE)` will be called after it returns, which may come as a 
surprise. If you want to ignore an exception, you need to catch it.
