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

noinst_PROGRAMS = src/genstaticlogo

src_dietsplash_SOURCES = src/dietsplash.c src/pnmtologo.c src/util.c \
			 src/pnmtologo.h src/util.h

src_genstaticlogo_SOURCES = src/genstaticlogo.c \
			    src/pnmtologo.c \
			    src/pnmtologo.h \
			    src/util.c \
			    src/util.h
