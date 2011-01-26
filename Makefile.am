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

CLEANFILES = src/background.h data/background.ppm \
	     $(nodist_systemunit_DATA)

rootbindir = @rootdir@/bin
systemunitdir = @rootdir@/lib/systemd/system/

rootbin_PROGRAMS = src/dietsplash

bin_PROGRAMS = src/dietsplashctl

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

AM_CPPFLAGS = \
	      -include $(top_builddir)/config.h \
	      -I$(builddir)/src -I$(srcdir)/src

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

src_dietsplashctl_SOURCES = src/dietsplashctl.c

if !ENABLE_STATICIMAGES
data_dietsplashdir = $(pkgdatadir)
data_dietsplash_DATA = $(background)
AM_CFLAGS += -DBACKGROUND_FILE=\""$(build_datadir)/background.ppm"\"

src/fb.o: makebackground

makebackground: $(background)
if MAINTAINER_MODE
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


EXTRA_DIST = data/default_background.ppm \
	     units/dietsplash-quit.service.in

nodist_systemunit_DATA = units/dietsplash-quit.service

SED_PROCESS = \
	      $(AM_V_GEN)$(MKDIR_P) $(dir $@) && \
	      $(SED) -e 's,@bindir\@,$(bindir),g' \
	      < $< > $@ || rm $@

units/%: units/%.in Makefile
	$(SED_PROCESS)


install-data-hook:
	$(MKDIR_P) -m 0755 \
		$(DESTDIR)$(systemunitdir)/multi-user.target.wants
	( cd $(DESTDIR)$(systemunitdir)/graphical.target.wants && \
		rm -f dietsplash-quit.service && \
		$(LN_S) ../dietsplash-quit.service dietsplash-quit.service )
if !ENABLE_STATICIMAGES
	( cd $(DESTDIR)$(data_dietsplashdir) && \
		rm -f background.ppm && \
		$(LN_S) $(shell basename ${background}) background.ppm )
endif
