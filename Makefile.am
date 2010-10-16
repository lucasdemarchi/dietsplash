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

CLEANFILES = src/logo.h

bin_PROGRAMS = src/dietsplash

if CUSTOM_LOGO
logo = @LOGO_PATH@
else
logo = $(abs_top_srcdir)/data/default_logo.ppm
endif

data_dietsplashdir = $(datadir)/@PACKAGE@

if MAINTAINER_MODE
build_datadir = $(abs_top_srcdir)/data
build_logo    = $(logo)
else
build_datadir = $(data_dietsplashdir)
build_logo    = $(build_datadir)/$(shell basename ${logo})
endif

AM_CFLAGS = -DDATADIR=\""$(build_datadir)"\"

src_dietsplash_SOURCES = src/dietsplash.c src/pnmtologo.c src/util.c \
			 src/pnmtologo.h src/util.h


if !STATICLOGO
data_dietsplash_DATA = $(logo)
EXTRA_DIST = $(data_dietsplash_DATA)

AM_CFLAGS += -DLOGOFILE=\""$(build_logo)"\"
else
noinst_PROGRAMS = src/genstaticlogo
src_dietsplash_SOURCES += src/logo.h

AM_CPPFLAGS = -I$(builddir)/src -I$(srcdir)/src

src_genstaticlogo_SOURCES = src/genstaticlogo.c \
			    src/pnmtologo.c \
			    src/pnmtologo.h \
			    src/util.c \
			    src/util.h

src/logo.h: $(logo) src/genstaticlogo
	$(AM_V_GEN)src/genstaticlogo $< $@

src/dietsplash.c: src/logo.h
endif
