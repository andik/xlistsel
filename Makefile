# ------------------------------------------------------------------------
# Globals
# ------------------------------------------------------------------------

NAME = xlistsel
VERSION = 0.8.1

SRC  = src/main.c  src/util.c src/style.c 
SRC += generated/options.c 
SRC += vendor/listview.c vendor/nkapp.c
OBJ = $(SRC:.c=.o)
BIN = $(NAME)

OPTGEN=tools/options.sh

MANPAGE=$(NAME).1
MANPAGE_OPTS=generated/$(MANPAGE).options-inc

RELEASE=$(NAME)-$(VERSION).tar.gz
HEADERS=extern.h vendor/listview.h vendor/nkapp.h

GENERATED=generated/options.h generated/options.c 

RELEASE_FILES=src/* doc/* examples/* tools/* vendor/*
RELEASE_FILES+=options.conf Readme.md config.mk Makefile COPYING

.include <config.mk>

.SUFFIXES: .c .o .md .html

# ------------------------------------------------------------------------
# Entry Point
# ------------------------------------------------------------------------

all: $(BIN) $(MANPAGE)
	
# ------------------------------------------------------------------------
# Doc
# ------------------------------------------------------------------------

$(MANPAGE_OPTS): $(OPTGEN)
	./$(OPTGEN) mdoc > $(MANPAGE_OPTS)
$(MANPAGE): src/$(MANPAGE).in $(MANPAGE_OPTS)
	$(CC) -E -P -w -x c src/$(MANPAGE).in -o $@

$(NAME).txt: $(MANPAGE)
	mandoc -T utf8 $(MANPAGE) > $@
$(NAME).html: $(MANPAGE)
	mandoc -T html $(MANPAGE) > $@
	
# this is only for testing purposes (e.g. viewing in browser before uploading)
.md.html:
	lowdown $< > $@

# ------------------------------------------------------------------------
# Generated Source
# ------------------------------------------------------------------------

$(OPTGEN): options.conf
	@touch $@

generated:
	mkdir -p $@

generated/options.h: $(OPTGEN) generated
	@./$(OPTGEN) decl > $@

generated/options.c: $(OPTGEN) generated
	@rm -f $@
	@echo '#include <stdio.h>' >> $@
	@echo '#include <stdlib.h>' >> $@
	@echo '#include "../src/extern.h"' >> $@
	@echo '#include "options.h"' >> $@
	@echo '#include <unistd.h>' >> $@
	@echo '#include <string.h>' >> $@
	@echo '#include <err.h>' >> $@
	@echo '' >> $@
	@./$(OPTGEN) help >> $@
	@echo '' >> $@
	@./$(OPTGEN) init >> $@
	@echo '' >> $@
	@./$(OPTGEN) parse >> $@
	@echo '' >> $@

	
# ------------------------------------------------------------------------
# Compile
# ------------------------------------------------------------------------

.c.o: $(GENERATED)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	rm -f $(BIN)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

# ------------------------------------------------------------------------
# Management
# ------------------------------------------------------------------------

install: $(BIN) $(MANPAGE)
	$(INSTALLSCRIPT) $(BIN) $(PREFIX)/bin/$(BIN)
	$(INSTALLDATA)   $(MANPAGE) $(PREFIX)/man/man1/$(MANPAGE)
	
clean:
	rm -f $(OBJ)
	rm -f $(GENERATED)
	rm -f $(MANPAGE)
	rm -f $(NAME).txt $(NAME).html Readme.html
	rm -f $(BIN)
	rm -f $(RELEASE)
	rm -f Makefile.bak
	
release: $(MANPAGE)
	rm -f $(MANPAGE_OPTS)
	tar czf $(RELEASE) $(RELEASE_FILES)

.PHONY:: install all clean

# ------------------------------------------------------------------------
# Development
# ------------------------------------------------------------------------

# build and show the manpage
man: $(MANPAGE)
	mandoc -a $(MANPAGE)
	
# run a very simple example
run: $(BIN)
	@-./$(BIN) -b B1 -b B2 -T 'Col 1:Col 2:Col 3' A1:B1:C1 A2:B2:C2 A3:B2:C3 ; echo $$?
	
deps: depend
depend: $(GENERATED)
	makedepend -- $(CFLAGS) -- $(SRC)
	
doc: Readme.html $(NAME).txt $(NAME).html
	
.PHONY:: man run depend doc deps

# ------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------
#
# These are built by calling 'make depend'.
# Do not modify below.
#

# DO NOT DELETE

