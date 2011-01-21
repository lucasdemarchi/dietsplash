ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
config.guess \
config.sub \
configure \
depcomp \
install-sh \
missing

CLEANFILES = src/background.h data/background.ppm

rootbindir = @rootdir@/bin

rootbin_PROGRAMS = src/dietsplash

if ENABLE_CUSTOM_BACKGROUND
background = @BACKGROUND_PATH@
else
background = $(abs_top_srcdir)/data/default_background.ppm
endif

if MAINTAINER_MODE
build_datadir = $(abs_top_builddir)/data
build_bindir = $(abs_top_builddir)/src
AM_CFLAGS = -DLOGLEVEL=4
else
build_datadir = $(data_dietsplashdir)
build_bindir = $(rootbindir)
AM_CFLAGS = -DLOGLEVEL=0
endif

AM_CFLAGS += -DDIETSPLASH_BIN=\""$(build_bindir)/dietsplash"\" \
            -DREAL_INIT=\""@REAL_INIT@"\"

if ENABLE_UMOUNTDEV
AM_CFLAGS += -DENABLE_UMOUNTDEV=1
endif

if ENABLE_LOG
AM_CFLAGS += -DENABLE_LOG=1
endif

AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src

src_dietsplash_SOURCES = \
			 src/events.c \
			 src/events.h \
			 src/fb.c \
			 src/fb.h \
			 src/log.c \
			 src/log.h \
			 src/main.c \
			 src/pnmtologo.c \
			 src/pnmtologo.h \
			 src/util.c \
			 src/util.h

if !ENABLE_STATICIMAGES
data_dietsplashdir = $(datadir)/@PACKAGE@
data_dietsplash_DATA = $(background)
AM_CFLAGS += -DBACKGROUND_FILE=\""$(build_datadir)/background.ppm"\"

if !MAINTAINER_MODE
install-data-hook:
	( cd $(DESTDIR)$(data_dietsplashdir) && \
		rm -f background.ppm && \
		$(LN_S) $(shell basename ${background}) background.ppm )
else
src/fb.o: makebackground

makebackground: $(background)
	$(AM_V_GEN) \
		( cd $(build_datadir) && rm -f background.ppm && \
		$(LN_S) $(background) background.ppm )
endif
else
noinst_PROGRAMS = src/genstaticlogo

src_genstaticlogo_SOURCES = \
			    src/genstaticlogo.c \
			    src/pnmtologo.c \
			    src/pnmtologo.h \
			    src/util.c \
			    src/util.h

src/background.h: $(background) src/genstaticlogo
	$(AM_V_GEN)src/genstaticlogo dietsplash_static_background $@ $<

src/fb.o: src/background.h
endif

EXTRA_DIST = $(srcdir)/data/default_background.ppm
