EXTRA_DIST = scripts autogen.sh
SUBDIRS = include src
ACLOCAL_AMFLAGS = --install -I m4

if NO_CVSBUILD
PKG_DISTDIR=$(PACKAGE)-$(PACKAGE_VERSION)
else
PKG_DISTDIR=$(PACKAGE)-$(CVSBUILD)
endif

distdir=$(PKG_DISTDIR)

DISTCLEANFILES = confdefs.h
