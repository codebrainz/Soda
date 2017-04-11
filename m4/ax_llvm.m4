dnl
dnl AX_CHECK_LLVM([components], [version])
dnl
dnl components - list of LLVM components or defaults to all
dnl version    - version of LLVM or empty for default
dnl
AC_DEFUN([AX_CHECK_LLVM], [
    AS_IF([test "x$1" = "x"],
        [llvm_components="all"],
        [llvm_components="$1"])
    AS_IF([test "x$2" = "x"],
        [llvm_config_tool="llvm-config"],
        [llvm_config_tool="llvm-config-$2"])
    AC_CHECK_TOOL([LLVM_CONFIG], [$llvm_config_tool], [
        AC_MSG_ERROR([unable to locate $llvm_config_tool utility])
    ])
    LLVM_CFLAGS=`$LLVM_CONFIG --cxxflags`
    LLVM_LIBS=`$LLVM_CONFIG --ldflags --libs $llvm_components`
])
