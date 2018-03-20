PREFIX ?= /usr/local
CFLAGS ?= -Wall -Wextra -pedantic

target := uoyweek

.PHONY: all run install clean

%: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

all: $(target)

run: $(target)
	./$(target) $(ARGS)

install: $(target)
	mkdir -p $(PREFIX)/bin
	cp $^ $(PREFIX)/bin

clean:
	$(RM) $(target)
