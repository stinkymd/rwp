CC = cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -pedantic
LDFLAGS =

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

TARGET = rwp
SRC = rwp.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

config.h: config.def.h
	cp config.def.h $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

rwp.o: rwp.c config.h
	$(CC) $(CFLAGS) -c -o $@ rwp.c

clean:
	rm -f $(TARGET) $(OBJ)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

.PHONY: all clean install uninstall
