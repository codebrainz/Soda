dnl
dnl AX_PLATFORM
dnl
AC_DEFUN([AX_PLATFORM], [
    AC_MSG_CHECKING([whether compiling for win32])
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([[
#ifndef _WIN32
# error "this is not win32"
#endif
        ]])], [
            platform_is_win32="yes"
            AC_MSG_RESULT([yes])
        ], [
            platform_is_win32="no"
            AC_MSG_RESULT([no])
        ])
    AS_IF([test "x$platform_is_win32" = "xyes"],
        [AC_CHECK_HEADERS([windows.h])],
        [AC_CHECK_HEADERS([fcntl.h sys/mman.h sys/stat.h unistd.h])])
    AM_CONDITIONAL([HAVE_WIN32], [test "x$platform_is_win32" = "xyes"])
])
