
VPATH=src:include:build
CFLAGS=-Iinclude -Wall -fPIC -ggdb
LDFLAGS=-shared

NAME=libmiscellany.so
MODULES=btree list except array map
TARGETS=$(addsuffix .o, $(MODULES))
HEADERS=$(addsuffix .h, $(MODULES))

BUILDDIR=build

.PHONY: clean install

$(NAME): $(TARGETS)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

$(TARGETS): | $(BUILDDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)

clean: 
	rm $(BUILDDIR)/*
	rm $(NAME)

install: $(NAME)
	install -Dm755 $(NAME) $(prefix)/lib/$(NAME)
	install -d $(prefix)/include/misc
	install -Dm644 $(addprefix include/, $(HEADERS)) $(prefix)/include/misc

$(addprefix $(BUILDDIR)/, $(TARGETS)): $(BUILDDIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
