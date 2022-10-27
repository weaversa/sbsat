EXTRAS = Makefile LICENSE.md README.md AUTHORS.md

HEADERS := $(shell find include src -type f -name '*.h')
SOURCES := $(shell find src -type f -name '*.c')
OBJECTS = $(SOURCES:src/%.c=obj/%.o)

HEADER_DIRS := $(shell find include src -type d)

SBSATLIB = sbsat
CC = gcc
DBG = #-g -Wall -fstack-protector-all -pedantic
OPT = -march=native -O3 -DNDEBUG -ffast-math -fomit-frame-pointer -finline-functions
INCLUDES = $(addprefix -I ,$(HEADER_DIRS))
LIBS = -l$(SBSATLIB) -lm
LDFLAGS = -Llib
CFLAGS = -std=gnu99 $(DBG) $(OPT) $(INCLUDES)
AR = ar r
RANLIB = ranlib

all: depend lib/lib$(SBSATLIB).a

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

lib/cudd/lib/libcudd.a:
	@cd lib/cudd && $(MAKE)

lib/picosat/lib/libpicosat.a:
	@cd lib/picosat && $(MAKE)

$(OBJECTS): obj/%.o : src/%.c Makefile
	@echo "Compiling "$<
	@[ -d $(dirname $@)] || mkdir -p $(dirname $@)
	@$(CC) $(CFLAGS) -c $< -o $@

lib/lib$(SBSATLIB).a: $(OBJECTS) Makefile lib/cudd/lib/libcudd.a lib/picosat/lib/libpicosat.a
	@echo "Creating "$@""
	@[ -d lib ] || mkdir -p lib
	@rm -f $@
	@cp lib/cudd/lib/libcudd.a $@
	@cp lib/picosat/lib/libpicosat.a $@
	@$(AR) $@ $(OBJECTS)
	@$(RANLIB) $@

test/test: test/test.c lib/lib$(SBSATLIB).a
	$(CC) $(CFLAGS) $(LDFLAGS) test/test.c -o test/test $(LIBS)

clean:
	cd lib/cudd && $(MAKE) clean
	cd lib/picosat && $(MAKE) clean
	rm -rf $(OBJECTS_CTHREADPOOL)
	rm -rf *~ */*~ $(OBJECTS) .depend test/test *.dSYM test/test.dSYM sbsat.tar.gz lib/lib$(SBSATLIB).a obj

edit:
	emacs -nw $(SOURCES) $(HEADERS) $(EXTRAS)
