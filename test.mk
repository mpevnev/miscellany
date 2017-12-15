
BUILDDIR=build
VPATH=src:$(BUILDDIR)
CFLAGS=-Wall -Iinclude -I../../include -ggdb `pkg-config --cflags check`
LDFLAGS=-L../../
LDLIBS=-lmiscellany `pkg-config --libs check`

TARGETS=main.o
TARGETS:=$(addprefix $(BUILDDIR)/, $(TARGETS))

$(NAME): $(TARGETS)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

$(TARGETS): | $(BUILDDIR)

$(BUILDDIR):
	 mkdir $(BUILDDIR)

clean: 
	rm $(BUILDDIR)/*
	rm $(NAME)

$(TARGETS): $(BUILDDIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
