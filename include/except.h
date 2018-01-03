#ifndef EXCEPT_H
#define EXCEPT_H

/** Exception module.
 *
 * Note that the whole exception mechanism is based on setjmp, so make any 
 * local variable that you want to use inside an EXCEPT block 'volatile'. 
 * Otherwise, undefined behaviour occurs.
 *
 * Any exception with namespace=0 is reserved for this library.
 *
 * Disposing of a caught exception is up to the user of the library. Just don't 
 * do it with built-in exceptions, please.
 */

#include <setjmp.h>
#include <stdlib.h>

#include "list.h"

/* ---------- base things ---------- */

/* I would personally prefer 'exception', but that's already taken by math.h. */
struct except
{
	int namespace;
	int type;
	void *data;
	void (*data_cleaner)(void *);
};

enum builtin_exc
{
	EXC_OOM
};

/* A handler *must* be noreturn. The library calls 'exit(EXIT_FAILURE)' right
 * after handler's invokation to placate the compiler, but please do make your
 * handlers noreturn. */
typedef void (*exc_unc_handler_t)(struct except*);

extern _Thread_local struct list *exc_try_points;
extern _Thread_local exc_unc_handler_t exc_unc_handler;

extern _Thread_local struct except *exc_exception;
extern _Thread_local struct except *exc_oom_exception;

/* ---------- initialization and shutdown ---------- */

/* This should be called in every thread that uses exceptions, once. 
 * Return 1 on success, 0 on failure. */
extern int
exc_init(void);

/* Call this if you wish to clean up memory used by the library. */
extern void
exc_fin(void);

extern void
exc_set_uncaught_handler(exc_unc_handler_t func);

/* ---------- operations ---------- */

extern struct except *
exc_create(int ns, int type, void *data, void (*data_cleaner)(void *));

extern _Noreturn void
exc_throw(struct except *exc);

/* If there's not enough memory to create a new exception, an OOM exception 
 * will be thrown instead. */
extern _Noreturn void
exc_throw_new(int ns, int type, void *data, void (*data_cleaner)(void *));

/* Clean exception's data and throw it again with new data. */
extern _Noreturn void
exc_rethrow(struct except *exc, void *new_data, void (*new_cleaner)(void *));

extern void *
exc_data(struct except *exc);

extern void
exc_destroy(struct except *);

#define TRY \
{ \
	jmp_buf *__try_point = malloc(sizeof(jmp_buf)); \
	if (__try_point == NULL) exc_throw(exc_oom_exception); \
	if (!list_push(exc_try_points, __try_point)) { \
		free(__try_point);  \
		exc_throw(exc_oom_exception); \
	} \
	volatile int __caught = 0; \
	volatile int __thrown = 0; \
	if (!setjmp(*__try_point)) { \

#define START_EXCEPT \
	} else { /* This closes !setjmp. */ \
		__thrown = 1; \
		{ /* This opens a dummy block to avoid special cases for the first \
		     EXCEPT. */ \

/* Caught exception will be available via variable named in the third argument. */
#define EXCEPT(__namespace, __type, __exc) \
	} /* This closes previous EXCEPT's 'if'. */ \
	if (!__caught && exc_exception->namespace == (__namespace) \
			&& exc_exception->type == (__type)) { \
		__caught = 1; \
		struct except *__exc = exc_exception; \
		(void) __exc; \

#define END_TRY \
	} /* This closes previous EXCEPT. */ \
	} /* This - the whole (if (!setmp) ... else). */ \
	if (!__thrown) { \
		struct jmp_buf *__try_point = list_pop(exc_try_points); \
		free(__try_point); \
	} \
	if (!__caught) exc_throw(exc_exception); \
} /* And finally this closes the entire TRY block. */

#endif /* EXCEPT_H */
