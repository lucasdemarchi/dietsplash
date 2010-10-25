AC_INIT(dietsplash, 0.1, [lucas.demarchi@profusion.mobi])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax subdir-objects dist-bzip2])
AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE

AC_PREFIX_DEFAULT(/usr)

AC_LANG_C
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_LN_S

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

AC_ARG_ENABLE(umountdev, AC_HELP_STRING([--enable-umountdev],
	                                 [unmount /dev if it was mounted by dietsplash]),
		[enable_umountdev=${enableval}], [enable_umountdev="no"])
AM_CONDITIONAL(UMOUNTDEV, test "${enable_umountdev}" == "yes")

AC_ARG_ENABLE(staticlogo, AC_HELP_STRING([--disable-staticlogo],
	                                 [disable static default logo]),
		[enable_staticlogo=${enableval}], [enable_staticlogo="yes"])
AM_CONDITIONAL(STATICLOGO, test "${enable_staticlogo}" != "no")

AC_ARG_WITH(logo, AC_HELP_STRING([--with-logo=LOGO],
	                         [specify location of logo to use or "default" for default logo]),
		[logo=${withval}], [logo="default"])
if (test "${logo}" != "default"); then
	LOGO_PATH="${logo}"
	AC_SUBST(LOGO_PATH)
fi
AM_CONDITIONAL(CUSTOM_LOGO, test "${logo}" != "default")

AC_ARG_WITH(init, AC_HELP_STRING([--with-realinit=INIT],
	                         [specify location of real init. Default is "/bin/systemd"]),
		[realinit=${withval}], [realinit="/bin/systemd"])
REAL_INIT="${realinit}"
AC_SUBST(REAL_INIT)

AC_OUTPUT(Makefile)
