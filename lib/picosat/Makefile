EXTRAS =

HEADERS = include/picosat.h

SOURCES = src/picosat.c

OBJECTS = $(SOURCES:src/%.c=obj/%.o)

PICOSATLIB = picosat
CC = gcc
DBG = #-g -Wall -fstack-protector-all -pedantic
OPT = -march=native -O3 -DNDEBUG -ffast-math -fomit-frame-pointer -finline-functions
INCLUDES = -Iinclude
LIBS = -l$(PICOSATLIB) -lm
LDFLAGS = -Llib
CFLAGS = -std=gnu99 $(DBG) $(OPT) $(INCLUDES)
AR = ar r
RANLIB = ranlib

all: depend lib/lib$(PICOSATLIB).a

depend: .depend
.depend: $(SOURCES)
	@echo "Building dependencies" 
ifneq ($(wildcard ./.depend),)
	@rm -f "./.depend"
endif
	@$(CC) $(CFLAGS) -MM $^ > .depend
# Make .depend use the 'obj' directory
	@sed -i.bak -e :a -e '/\\$$/N; s/\\\n//; ta' .depend
	@sed -i.bak 's/^/obj\//' .depend
	@rm -f .depend.bak
-include .depend

$(OBJECTS): obj/%.o : src/%.c Makefile
	@echo "Compiling "$<""
	@[ -d obj ] || mkdir -p obj
	@$(CC) $(CFLAGS) -c $< -o $@

lib/lib$(PICOSATLIB).a: $(OBJECTS) Makefile
	@echo "Creating "$@""
	@[ -d lib ] || mkdir -p lib
	@rm -f $@
	@$(AR) $@ $(OBJECTS)
	@$(RANLIB) $@

.PHONY: dist
dist: $(SOURCES) $(HEADERS) $(EXTRAS)
	rm -rf picosat.tar.gz
	[ -d picosat         ] || mkdir -p picosat
	[ -d picosat/src     ] || mkdir -p picosat/src
	[ -d picosat/include ] || mkdir -p picosat/include
	rsync -R $(SOURCES) $(HEADERS) $(EXTRAS) picosat
	COPYFILE_DISABLE=1 tar cvzf picosat.tar.gz picosat
	rm -rf picosat

clean:
	rm -rf *~ */*~ $(OBJECTS) ./.depend *.dSYM picosat.tar.gz lib/lib$(PICOSATLIB).a obj

edit:
	emacs -nw $(SOURCES) $(HEADERS) $(EXTRAS)
