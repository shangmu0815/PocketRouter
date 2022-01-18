#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([PocketRouter],1.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AS_CASE([$host],
    [arm*], [ARM=yes],
    [ARM=no]
)

AC_ARG_ENABLE(customer-code,
  [AS_HELP_STRING([--enable-customer-code], [Specify the customer to build])],
  [customer_code=$enableval],
  [customer_code=default]
)
AM_CONDITIONAL([IS_DEFAULT], [test "x$customer_code" = "xdefault"])
AM_CONDITIONAL([IS_DLINK], [test "x$customer_code" = "xdlink"])
AM_CONDITIONAL([IS_SPEEDTEST], [test "x$customer_code" = "xdlink"])
AM_CONDITIONAL([IS_ZYXEL], [test "x$customer_code" = "xzyxel"])
AM_CONDITIONAL([IS_ONDA], [test "x$customer_code" = "xonda"])
AM_CONDITIONAL([IS_GEMTEKS], [test "x$customer_code" = "xgemteks"])
AM_CONDITIONAL([IS_ZX73_DEFAULT], [test "x$customer_code" = "xzx73default"])
AM_CONDITIONAL([IS_ZX73_NEC], [test "x$customer_code" = "xzx73nec"])
AM_CONDITIONAL([IS_ZX73_APAL], [test "x$customer_code" = "xzx73apal"])
AM_CONDITIONAL([IS_UPURPLE], [test "x$customer_code" = "xupurple"])
AM_CONDITIONAL([IS_JARVISH], [test "x$customer_code" = "xjarvish"])

# compile options
AC_ARG_ENABLE(bt-support,
  [AS_HELP_STRING([--enable-bt-support], [Specify if enable bt support])],
  [bt_support=$enableval],
  [bt_support=default]
)
AM_CONDITIONAL([IS_BT_SUPPORT], [test "x$bt_support" = "xyes"])

AC_ARG_ENABLE(boot-animation,
  [AS_HELP_STRING([--enable-boot-animation], [Specify if add boot animation])],
  [boot_animation=$enableval],
  [boot_animation=default]
)
AM_CONDITIONAL([IS_BOOT_ANIMATION], [test "x$boot_animation" = "xyes"])

# QMI-Framework for QCCI APIs
PKG_CHECK_MODULES([QMIF], [qmi-framework])
AC_SUBST([QMIF_CFLAGS])
AC_SUBST([QMIF_LIBS])

# QMI/QMUX for EXT_QMUX support
PKG_CHECK_MODULES([QMI], [qmi])
AC_SUBST([QMI_CFLAGS])
AC_SUBST([QMI_LIBS])

#AM_CONDITIONAL(ARM, [test "x$ARM" = "xyes"])
AC_SUBST([CFLAGS])
AC_SUBST([CC])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
