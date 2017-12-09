
VPATH=src:include:build
CFLAGS=-Iinclude -Wall -fPIC -ggdb
LDFLAGS=-shared

NAME=libmiscellany.so
BUILDDIR=build
TARGETS=btree.o dlist.o

.PHONY: clean

$(NAME): $(TARGETS)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

clean: 
	rm $(BUILDDIR)/*
	rm $(NAME)

$(addprefix $(BUILDDIR)/, $(TARGETS)): $(BUILDDIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
