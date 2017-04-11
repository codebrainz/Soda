AC_INIT([Soda], [0.1])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([Soda/Config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AX_CXX_COMPILE_STDCXX_14([noext], [mandatory])
AX_CHECK_LLVM([all])
AX_PLATFORM
AC_CONFIG_FILES([Makefile Soda/Makefile])
AC_OUTPUT
