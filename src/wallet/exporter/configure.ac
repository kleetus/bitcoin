AC_PREREQ([2.69])
AC_INIT([bitcoin_wallet_exporter], [0.0.1], [chrisk@bitpay.com])
AC_OUTPUT(Makefile src/Makefile)
AC_CONFIG_SRCDIR([src/exporter.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(db, db_create, LIBS="$LIBS -ldb", 
  AC_MSG_ERROR([Can not locate a suitable BerkeleyDB library.]))

AC_MSG_CHECKING(BerkeleyDB)
AC_TRY_LINK_FUNC(db_create,, AC_MSG_ERROR([BerkeleyDB v4.8 package is not working properly.]))
AC_MSG_RESULT(BerkeleyDB)

# Enable tests 
AC_ARG_ENABLE([tests],
  [AS_HELP_STRING([--enable-tests],
  [enable tests (disabled by default)])],
  [use_tests=$enableval],
  [use_tests=no])

AC_MSG_CHECKING([whether to build exporter_test])
if test x$use_tests = xyes; then
  AC_MSG_RESULT([yes])
  BUILD_TEST="yes"
else
  AC_MSG_RESULT([no])
  BUILD_TEST=""
fi

if test x$use_tests != xno; then
  AC_CHECK_LIB(crypto, EVP_CIPHER_CTX_init, LIBS="$LIBS -lcrypto", 
               AC_MSG_ERROR([Can not locate a crypto library. Usually OpenSSL-based.]))
  AC_MSG_CHECKING(Crypto)
  AC_TRY_LINK_FUNC(EVP_CIPHER_CTX_init,, AC_MSG_ERROR([A crypto library support EVP functions was not found.]))
  AC_MSG_RESULT(Crypto)
fi
# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AM_CONDITIONAL([ENABLE_TESTS],[test x$BUILD_TEST = xyes])
AC_OUTPUT
