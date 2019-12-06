PREFIX ?= /usr/local
CFLAGS ?= -Wall -Wextra -pedantic

target := uoyweek

.PHONY: all run install clean

all: $(target)

run: $(target)
	./$(target) $(ARGS)

install: $(target)
	mkdir -p $(PREFIX)/bin
	cp $^ $(PREFIX)/bin

clean:
	$(RM) $(target)
