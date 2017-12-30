#ifndef EXCEPT_H
#define EXCEPT_H

/** Exception module.
 *
 * Note that catching exceptions uses 'setjmp' under the hood, so if you want
 * to use some local variable inside a CATCH block, make it 'volatile'.
 * Otherwise, it's undefined behaviour.
 *
 * Any exception with namespace=0 is reserved for this library.
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
	volatile int __popped = 0; \
	if (!setjmp(*__try_point)) { \

#define START_EXCEPT \
	} else { { \

/* Exception's data will be available via a 'void *' variable with the name
 * passed as the third argument to EXCEPT. */
#define EXCEPT(__namespace, __type, __data) \
	} \
	if (!__caught && exc_exception->namespace == (__namespace) \
			&& exc_exception->type = (__type)) { \
		__caught = 1; \
		void *__data = exc_exception->data;

#define END_TRY \
	} } \
	if (!__popped) { free(list_pop(exc_try_points)); __popped = 1; } \
	if (!__caught) exc_throw(exc_exception); \
}

#endif /* EXCEPT_H */
