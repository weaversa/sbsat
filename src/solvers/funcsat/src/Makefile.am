AM_CFLAGS = $(CODEMODEL) $(SHARED_CFLAGS) $(OPT_CFLAGS) $(DEBUG_CFLAGS) $(WARN_CFLAGS) $(PROF_CFLAGS)
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)
AM_LDFLAGS = $(CODEMODEL)

nobase_include_HEADERS = funcsat/config.h \
  funcsat/hashtable_private.h funcsat/hashtable.h funcsat/hashtable_utility.h funcsat/hashtable_itr.h funcsat/vecgen.h \
  funcsat/fibheap.h funcsat/fibheap_internal.h \
  funcsat/system.h

include_HEADERS = funcsat.h funcsat_internal.h


# funcsat/types.h funcsat/system.h funcsat/clause.h 
# funcsat/memory.h 
# funcsat/vec.h funcsat/vecgen.h funcsat/clausevec.h 
# funcsat/list.h 
# funcsat/fifoq.h 
# funcsat/fibheap.h funcsat/fibheap_internal.h 
# funcsat/debug.h 
# funcsat/posvec.h funcsat/intvec.h 
# funcsat/binwatch.h 
# funcsat/internal.h 
# funcsat/proof.h 
# learning.h 
# funcsat/restart.h 
# funcsat/frontend.h 
# funcsat/backbone.h


noinst_LTLIBRARIES = libfuncsat.la
libfuncsat_la_SOURCES = \
  util/hashtable.c util/hashtable_utility.c util/hashtable_itr.c util/fibheap.c \
  funcsat.c \
  funcsat/vec_reason_info.h funcsat/vec_uintptr.h funcsat/vec_clause_head.h \
  funcsat/vec_clause_block.h

util/vec.c: funcsat.h
util/fibheap.c: funcsat.h
funcsat/vec_reason_info.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/reason_info/g' funcsat/vecgen.h.in | sed 's/@TYPE@/struct reason_info/g' >$@
funcsat/vec_uintptr.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/uintptr/g' funcsat/vecgen.h.in | sed 's/@TYPE@/uintptr_t/g' >$@
funcsat/vec_clause_head.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/clause_head/g' funcsat/vecgen.h.in | sed 's/@TYPE@/struct clause_head/g' >$@
funcsat/vec_clause_block.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/clause_block/g' funcsat/vecgen.h.in | sed 's/@TYPE@/struct clause_block/g' >$@
funcsat/vec_clause.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/clause/g' funcsat/vecgen.h.in | sed 's/@TYPE@/clause */g' >$@
funcsat/vec_char.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/char/g' funcsat/vecgen.h.in | sed 's/@TYPE@/char/g' >$@
funcsat/vec_ptr.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/ptr/g' funcsat/vecgen.h.in | sed 's/@TYPE@/void */g' >$@
funcsat/vec_bool.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/bool/g' funcsat/vecgen.h.in | sed 's/@TYPE@/bool/g' >$@
funcsat/vec_head_tail.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/head_tail/g' funcsat/vecgen.h.in | sed 's/@TYPE@/head_tail/g' >$@
funcsat/vec_uintmax.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/uintmax/g' funcsat/vecgen.h.in | sed 's/@TYPE@/uintmax_t/g' >$@
funcsat/vec_intmax.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/intmax/g' funcsat/vecgen.h.in | sed 's/@TYPE@/intmax_t/g' >$@
funcsat/vec_uint64.h: funcsat/vecgen.h.in
	sed 's/@SHORTNAME@/uint64/g' funcsat/vecgen.h.in | sed 's/@TYPE@/uint64_t/g' >$@

# dist_doc_DATA = funcsat.pdf

# learning.c funcsat/learning.h: learning.w
# 	$(CTANGLE) $^ - learning.c
# 	mv learning.h funcsat/
# learning.h: learning.c


EXTRA_DIST = funcsat.w funcsat.tex funcsat/vecgen.h.in

funcsat.w: funcsat/vec_reason_info.h funcsat/vec_uintptr.h \
  funcsat/vec_clause_head.h funcsat/vec_clause_block.h \
  funcsat/vec_char.h funcsat/vec_bool.h funcsat/vec_ptr.h \
  funcsat/vec_uintmax.h funcsat/vec_intmax.h funcsat/vec_uint64.h
funcsat.c funcsat.h funcsat_internal.h: funcsat.w
	$(CTANGLE) $< - $@
funcsat.h: funcsat.c
funcsat_internal.h: funcsat.h

# %.c: %.w
# 	$(CTANGLE) $< - $@
funcsat.tex: funcsat.w
	$(CWEAVE) $< - $@
funcsat.pdf: funcsat.tex
	$(PDFTEX) -file-line-error $<
	$(PDFTEX) -file-line-error $<


