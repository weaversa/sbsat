EXTRAS = Makefile LICENSE README RELEASE.notes

HEADERS = include/mtr.h include/ntr.h include/st.h			\
include/cstringstream.h include/dddmpInt.h include/bnet.h		\
include/mtrInt.h include/cudd.h include/util.h include/cuddInt.h	\
include/epdInt.h include/epd.h include/dddmp.h

SOURCES = src/cuddZddIsop.c src/cuddClip.c src/chkMterm.c		\
src/cuddExact.c src/mtrBasic.c src/cuddLinear.c src/texpand.c		\
src/mtrGroup.c src/cuddAddFind.c src/cuddZddSymm.c src/cuddCompose.c	\
src/ntrShort.c src/cuddReorder.c src/dddmpNodeBdd.c src/cuddUtil.c	\
src/cuddLevelQ.c src/dddmpDbg.c src/dddmpLoad.c src/cuddWindow.c	\
src/cuddZddCount.c src/cuddCache.c src/cuddSubsetHB.c			\
src/cuddLiteral.c src/dddmpNodeAdd.c src/cuddAddApply.c			\
src/pipefork.c src/ucbqsort.c src/cuddLCache.c src/ntrZddTest.c		\
src/cpu_stats.c src/cuddAddWalsh.c src/cuddInit.c src/cuddSolve.c	\
src/ntrHeap.c src/cuddInteract.c src/cuddMatMult.c src/cuddApa.c	\
src/cuddAnneal.c src/cuddZddUtil.c src/cuddAddAbs.c			\
src/dddmpStoreAdd.c src/cuddHarwell.c src/cuddExport.c			\
src/cuddDecomp.c src/cuddBridge.c src/cuddBddIte.c src/cuddSign.c	\
src/epd.c src/cuddGenetic.c src/dddmpStoreBdd.c src/cuddRead.c		\
src/cpu_time.c src/st.c src/cuddZddGroup.c src/safe_mem.c		\
src/cstringstream.c src/ntr.c src/dddmpBinary.c src/cuddRef.c		\
src/cuddZddMisc.c src/ntrBddTest.c src/cuddSplit.c src/strsav.c		\
src/cuddZddFuncs.c src/dddmpConvert.c src/cuddSymmetry.c		\
src/cuddApprox.c src/dddmpStoreCnf.c src/cuddAndAbs.c src/cuddSat.c	\
src/bnet.c src/cuddZddLin.c src/cuddAddIte.c src/cuddPriority.c		\
src/dddmpStoreMisc.c src/cuddCheck.c src/cuddAddNeg.c			\
src/cuddBddAbs.c src/prtime.c src/cuddAPI.c src/cuddTable.c		\
src/ntrMflow.c src/dddmpLoadCnf.c src/dddmpUtil.c src/cuddEssent.c	\
src/cuddZddSetop.c src/cuddBddCorr.c src/cuddCof.c src/pathsearch.c	\
src/dddmpNodeCnf.c src/cuddZddReord.c src/cuddGroup.c			\
src/cuddAddInv.c src/cuddZddPort.c src/cuddSubsetSP.c src/datalimit.c	\
src/cuddGenCof.c


OBJECTS = $(SOURCES:src/%.c=obj/%.o)

CUDDLIB = cudd
CC = gcc
DBG = #-g -Wall -fstack-protector-all -pedantic
OPT = -march=native -O3 -DNDEBUG -ffast-math -fomit-frame-pointer -finline-functions
INCLUDES = -Iinclude
LIBS = -l$(CUDDLIB) -lm
LDFLAGS = -Llib
CFLAGS = -std=gnu99 $(DBG) $(OPT) $(INCLUDES)
AR = ar r
RANLIB = ranlib

all: depend lib/lib$(CUDDLIB).a

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

lib/lib$(CUDDLIB).a: $(OBJECTS) Makefile
	@echo "Creating "$@""
	@[ -d lib ] || mkdir -p lib
	@rm -f $@
	@$(AR) $@ $(OBJECTS)
	@$(RANLIB) $@

.PHONY: dist
dist: $(SOURCES) $(HEADERS) $(EXTRAS)
	rm -rf cudd.tar.gz
	[ -d cudd         ] || mkdir -p cudd
	[ -d cudd/src     ] || mkdir -p cudd/src
	[ -d cudd/include ] || mkdir -p cudd/include
	rsync -R $(SOURCES) $(HEADERS) $(EXTRAS) cudd
	COPYFILE_DISABLE=1 tar cvzf cudd.tar.gz cudd
	rm -rf cudd

clean:
	rm -rf *~ */*~ $(OBJECTS) ./.depend *.dSYM cudd.tar.gz lib/lib$(CUDDLIB).a obj

edit:
	emacs -nw $(SOURCES) $(HEADERS) $(EXTRAS)
