
BUILDDIR=build
VPATH=src:$(BUILDDIR)
CFLAGS=-Wall -I../../include `pkg-config --cflags check`
LDFLAGS=-L../../
LDLIBS=-lmiscellany `pkg-config --libs check`

TARGETS=main.o

$(NAME): $(TARGETS)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

clean: 
	rm $(BUILDDIR)/*
	rm $(NAME)

$(addprefix $(BUILDDIR)/, $(TARGETS)): $(BUILDDIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
