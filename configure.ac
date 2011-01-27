AC_INIT([dietsplash],[0.1],[lucas.demarchi@profusion.mobi])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax subdir-objects dist-bzip2])
AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE

AC_PREFIX_DEFAULT(/usr/local)

AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
AC_USE_SYSTEM_EXTENSIONS

AC_PROG_LN_S
AC_PROG_SED
PKG_PROG_PKG_CONFIG

CC_CHECK_CFLAGS_APPEND([ \
        -pipe \
        -Wall \
        -W \
        -Wextra \
        -Wno-inline \
        -Wvla \
        -Wundef \
        -Wformat=2 \
        -Wlogical-op \
        -Wsign-compare \
        -Wformat-security \
        -Wmissing-include-dirs \
        -Wformat-nonliteral \
        -Wold-style-definition \
        -Wpointer-arith \
        -Winit-self \
        -Wdeclaration-after-statement \
        -Wfloat-equal \
        -Wmissing-prototypes \
        -Wstrict-prototypes \
        -Wredundant-decls \
        -Wmissing-declarations \
        -Wmissing-noreturn \
        -Wshadow \
        -Wendif-labels \
        -Wcast-align \
        -Wstrict-aliasing=2 \
        -Wwrite-strings \
        -Wno-long-long \
        -Wno-overlength-strings \
        -Wno-unused-parameter \
        -Wno-missing-field-initializers \
        -Wno-unused-result \
        -Wp,-D_FORTIFY_SOURCE=2 \
        -ffast-math \
        -fno-common \
        -fdiagnostics-show-option \
        -fno-strict-aliasing \
        -fvisibility=hidden \
        -ffunction-sections \
        -fdata-sections \
        -Wl,--as-needed \
        -Wl,--gc-sections])

################################# Umount /dev
AC_ARG_ENABLE(umountdev, AS_HELP_STRING([--enable-umountdev],
	        [unmount /dev if it was mounted by dietsplash]),
		[enable_umountdev=${enableval}])
if (test "${enable_umountdev}" == "yes"); then
	AC_DEFINE(ENABLE_UMOUNTDEV, 1, [Set to 1 if unmountdev is enabled])
fi

################################# Disable log
AC_ARG_ENABLE([log], AS_HELP_STRING([--disable-log], [Completely disable log,
	       even removing log strings from binary]),
	       [enable_log=${enableval}])
if (test "${enable_log}" != "no"); then
	AC_DEFINE(ENABLE_LOG, 1, [Set to 1 if log is enabled])
fi

################################# Static images
AC_ARG_ENABLE(staticimages, AS_HELP_STRING([--disable-staticimages],
	      [disable images to be converted in source files at compile time.]),
	      [enable_staticimages=${enableval}])
AM_CONDITIONAL(ENABLE_STATICIMAGES, test "${enable_staticimages}" != "no")

################################# Custom background
AC_ARG_WITH(bg, AS_HELP_STRING([--with-bg=BG_FILE],
	    [specify location of background image to use or "default" for
	     default background]), [bg=${withval}], [bg="default"])
if (test "${bg}" != "default"); then
	BACKGROUND_PATH="${bg}"
	AC_SUBST(BACKGROUND_PATH)
fi
AM_CONDITIONAL(ENABLE_CUSTOM_BACKGROUND, test "${bg}" != "default")

AC_ARG_WITH([systemdsystemunitdir],
	AS_HELP_STRING([--with-systemdsystemunitdir=DIR],  [Directory for systemd service files]),
	[], [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
if test "x$with_systemdsystemunitdir" != xno; then
    AC_SUBST([SYSTEMDSYSTEMUNITDIR], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(ENABLE_SYSTEMD, [test -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno ])

################################# Real init
AC_ARG_WITH(init, AS_HELP_STRING([--with-realinit=INIT],
	    [specify location of real init. Default is "/bin/systemd"]),
	    [realinit=${withval}],
	    [
	     if test -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno; then
		     realinit="/bin/systemd"
	     else
		     realinit="/sbin/init"
	     fi
	     ])
REAL_INIT="${realinit}"
AC_SUBST(REAL_INIT)


################################# Root dir
AC_ARG_WITH([rootdir],
        AS_HELP_STRING([--with-rootdir=DIR],
		       [Root directory for files necessary for boot]))
AC_SUBST([rootdir], [$with_rootdir])


#################################
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
