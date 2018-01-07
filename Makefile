
VPATH=src:include:build
CFLAGS=-Iinclude -Wall -fPIC -ggdb
LDFLAGS=-shared 
LDLIBS=-lm

NAME=libmiscellany.so
MODULES=btree list except array map
TARGETS=$(addsuffix .o, $(MODULES))
HEADERS=$(addsuffix .h, $(MODULES))
DOCS=$(addsuffix .md, $(MODULES))

BUILDDIR=build
INCDIR=include/misc
DOCDIR=share/doc/libmiscellany

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
	install -d $(prefix)/$(INCDIR)
	install -d $(prefix)/$(DOCDIR)
	install -Dm644 $(addprefix include/, $(HEADERS)) $(prefix)/$(INCDIR)
	install -Dm644 $(addprefix docs/, $(DOCS)) $(prefix)/$(DOCDIR)

$(addprefix $(BUILDDIR)/, $(TARGETS)): $(BUILDDIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
