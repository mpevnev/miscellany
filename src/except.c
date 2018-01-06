
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "except.h"
#include "list.h"

/* Helper things. */

void 
exc_destroy_v(void *exc);

void
default_exception_handler(struct except *);

/* ---------- data ---------- */

_Thread_local struct list *exc_try_points;
_Thread_local exc_unc_handler_t exc_unc_handler;

_Thread_local struct except *exc_exception;
_Thread_local struct except *exc_oom_exception;

/* ---------- initialization and shutdown ---------- */

int
exc_init(void)
{
	exc_try_points = list_create();
	if (exc_try_points == NULL)
		return 0;
	exc_unc_handler = &default_exception_handler;
	exc_exception = NULL;

	exc_oom_exception = exc_create(0, EXC_OOM, NULL, NULL);
	if (exc_oom_exception == NULL)
		return 0;

	return 1;
}

void
exc_fin(void)
{
	if (exc_try_points != NULL) {
		list_destroy_ex(exc_try_points, &exc_destroy_v);
		exc_try_points = NULL;
	}
	exc_destroy(exc_oom_exception);
	if (exc_exception != NULL && exc_exception->namespace != 0) 
		exc_destroy(exc_exception);
	exc_unc_handler = &default_exception_handler;
}

void
exc_set_uncaught_hander(exc_unc_handler_t func)
{
	exc_unc_handler = func;
}

/* ---------- operations ---------- */

struct except *
exc_create(int namespace, int type, void *data, void (*data_cleaner)(void *))
{
	struct except *res = malloc(sizeof(struct except));
	if (res == NULL) return NULL;
	res->namespace = namespace;
	res->type = type;
	res->data = data;
	res->data_cleaner = data_cleaner;
	return res;
}

_Noreturn void
exc_throw(struct except *exc)
{
	exc_exception = exc;
	if (list_empty(exc_try_points)) {
		exc_unc_handler(exc);
		exit(EXIT_FAILURE);
	} else {
		jmp_buf *try_point = list_pop(exc_try_points);
		jmp_buf copy;
	        memcpy(&copy, try_point, sizeof(jmp_buf));
		free(try_point);
		longjmp(copy, 1);
	}
}

_Noreturn void
exc_throw_new(int ns, int type, void *data, void (*data_cleaner)(void *))
{
	struct except *throw = exc_create(ns, type, data, data_cleaner);
	if (throw == NULL) throw = exc_oom_exception;
	exc_throw(throw);
}

_Noreturn void
exc_rethrow(struct except *exc, void *new_data, void (*new_cleaner)(void *))
{
	if (exc->data_cleaner != NULL)
		exc->data_cleaner(exc->data);
	exc->data = new_data;
	exc->data_cleaner = new_cleaner;
	exc_throw(exc);
}

extern void *
exc_data(struct except *exc);

void
exc_destroy(struct except *exc)
{
	if (exc->data_cleaner != NULL) 
		exc->data_cleaner(exc->data);
	free(exc);
}

/* ---------- helper things ---------- */

void
exc_destroy_v(void *exc)
{
	exc_destroy(exc);
}

_Noreturn void
default_exception_handler(struct except *exc)
{
	fprintf(stderr, "Uncaught exception %d:%d\n", exc->namespace, exc->type);
	exit(EXIT_FAILURE);
}
