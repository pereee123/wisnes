AC_INIT(mac_dvcs,0.1)
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_MAKE_SET

AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL


CFLAGS="${CFLAGS=} -Wall "

AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(CPP_NAMESPACE,std)

AC_PATH_NS_ALLINONE

AC_CONFIG_FILES([
		Makefile
		src/Makefile
		samples/Makefile
		m4/Makefile
 		])

AC_OUTPUT

