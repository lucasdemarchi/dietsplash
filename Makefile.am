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

CLEANFILES = src/logo.h data/logo.ppm

rootbindir = @rootdir@/bin

rootbin_PROGRAMS = src/dietsplash

if CUSTOM_LOGO
logo = @LOGO_PATH@
else
logo = $(abs_top_srcdir)/data/default_logo.ppm
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

if UMOUNTDEV
AM_CFLAGS += -DDEVTMPFS_UMOUNT_REQUIRED=1
endif

AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src

src_dietsplash_SOURCES = \
			 src/fb.c \
			 src/fb.h \
			 src/log.c \
			 src/log.h \
			 src/main.c \
			 src/pnmtologo.c \
			 src/pnmtologo.h \
			 src/util.c \
			 src/util.h

if !STATICLOGO
data_dietsplashdir = $(datadir)/@PACKAGE@
data_dietsplash_DATA = $(logo)
AM_CFLAGS += -DLOGOFILE=\""$(build_datadir)/logo.ppm"\"

if !MAINTAINER_MODE
install-data-hook:
	( cd $(DESTDIR)$(data_dietsplashdir) && \
		rm -f logo.ppm && \
		$(LN_S) $(shell basename ${logo}) logo.ppm )
else
src/fb.o: makelogo

makelogo: $(logo)
	$(AM_V_GEN) \
		( cd $(build_datadir) && rm -f logo.ppm && \
		$(LN_S) $(logo) logo.ppm )
endif
else
noinst_PROGRAMS = src/genstaticlogo

src_genstaticlogo_SOURCES = \
			    src/genstaticlogo.c \
			    src/pnmtologo.c \
			    src/pnmtologo.h \
			    src/util.c \
			    src/util.h

src/logo.h: $(logo) src/genstaticlogo
	$(AM_V_GEN)src/genstaticlogo dietsplash_staticlogo $@ $<

src/fb.o: src/logo.h
endif

EXTRA_DIST = $(srcdir)/data/default_logo.ppm
