#                                               -*- Autoconf -*-
AC_PREREQ([2.64])

AC_INIT([heatmap],
        [m4_esyscmd_s([./get-version])],
        [admin@itdev.co.uk])
AC_CONFIG_SRCDIR([src/log.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/heatmap.8])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror tar-ustar])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

# Configure libtool
LT_INIT

#######################
### Checks

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_EGREP

# Check some compiler flags
AX_CFLAGS_GCC_OPTION([-fdiagnostics-show-option],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-pipe],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wall],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-W],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wextra],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wformat],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wformat-security],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wfatal-errors],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wcast-align],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Winline],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-fstack-protector],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-D_FORTIFY_SOURCE=2],[MORE_CPPFLAGS])
AX_CFLAGS_GCC_OPTION([-Wno-unused-parameter],[MORE_CFLAGS])
AX_CFLAGS_GCC_OPTION([-Wno-missing-field-initializers],[MORE_CFLAGS])
AX_LDFLAGS_OPTION([-Wl,-z,relro],[MORE_LDFLAGS])
AX_LDFLAGS_OPTION([-Wl,-z,now],[MORE_LDFLAGS])

dnl Enable _GNU_SOURCE and the like
AX_CFLAGS_GCC_OPTION([-D_GNU_SOURCE],[MORE_CPPFLAGS])    dnl GNU systems (asprintf, ...)
case $host_os in
   solaris*)
     AX_CFLAGS_GCC_OPTION([-D__EXTENSIONS__],[MORE_CPPFLAGS]) dnl (CMSG_*)
     AX_CFLAGS_GCC_OPTION([-D_XPG4_2],[MORE_CPPFLAGS])        dnl (CMSG_*)
     ;;
   hpux*)
     AX_CFLAGS_GCC_OPTION([-D_XOPEN_SOURCE=500],[MORE_CPPFLAGS])      dnl HP-UX
     AX_CFLAGS_GCC_OPTION([-D_XOPEN_SOURCE_EXTENDED],[MORE_CPPFLAGS]) dnl HP-UX
     ;;
esac

AC_CACHE_SAVE

AX_WITH_CURSES
if test "x$ax_cv_curses" != xyes || test "x$ax_cv_curses_color" != xyes || test "x$ax_cv_curses_obsolete" != xyes; then
    AC_MSG_ERROR([*** requires an X/Open-compatible Curses library with color])
fi

AC_CACHE_SAVE

hm_ARG_WITH([hm-default-rate],
            [Default refresh rate],
            [0])
hm_ARG_WITH([hm-default-width],
            [Default touchscreen width],
            [19])
hm_ARG_WITH([hm-default-max],
            [Default maximum value],
            [auto])
hm_ARG_WITH([hm-default-min],
            [Default minimum value],
            [auto])

AC_SUBST([MORE_CFLAGS])
AC_SUBST([MORE_CPPFLAGS])
AC_SUBST([MORE_LDFLAGS])
AC_OUTPUT

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  C Compiler.....: $CC $MORE_CFLAGS $MORE_CPPFLAGS $CFLAGS $CPPFLAGS
  Linker.........: $LD $MORE_LDFLAGS $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
