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

bin_PROGRAMS = src/dietsplash

if STATICLOGO
noinst_PROGRAMS = src/genstaticlogo
endif

data_dietsplashdir = $(datadir)/@PACKAGE@
data_dietsplash_DATA = data/default_logo.ppm
EXTRA_DIST = $(data_dietsplash_DATA)

if MAINTAINER_MODE
build_datadir = $(abs_top_srcdir)/data
else
build_datadir = $(data_dietsplashdir)
endif

AM_CFLAGS = -DDATADIR=\""$(build_datadir)"\"

src_dietsplash_SOURCES = src/dietsplash.c src/pnmtologo.c src/util.c \
			 src/pnmtologo.h src/util.h



if STATICLOGO
src_dietsplash_SOURCES += src/default_logo.c

AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src

src_genstaticlogo_SOURCES = src/genstaticlogo.c \
			    src/pnmtologo.c \
			    src/pnmtologo.h \
			    src/util.c \
			    src/util.h

src/default_logo.c: $(abs_top_srcdir)/data/default_logo.ppm src/genstaticlogo
	$(AM_V_GEN)src/genstaticlogo $< $@

endif
