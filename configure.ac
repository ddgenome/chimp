# Process this file with autoconf to produce a configure script.
AC_REVISION([$Id$])
AC_PREREQ(2.13)dnl               dnl for PACKAGE and VERSION
AC_INIT([chimp], [0.2], [banjo@users.sourceforge.net])
AC_CONFIG_SRCDIR([src/chimp.cc])
#AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_CPP
AM_PROG_LEX
AC_PROG_RANLIB
AC_PROG_YACC
AC_ARG_VAR([PERL], [path to Perl 5 or better interpreter])
AC_CHECK_PROG([PERL], [perl], [perl], [NULL])
AM_CONDITIONAL([PERLEXIST], [test x$PERL = xperl])

# Checks for libraries.
AC_CHECK_LIB([m], [exp])

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h malloc.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_STRUCT_TM
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T                    dnl this does gid_t also

if test -n "${GCC}"; then
   CFLAGS="${CFLAGS} -W -Wall"
fi
if test -n "${GXX}"; then
   :
   #CXXFLAGS="${CXXFLAGS} -W -Wall"
fi

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([strerror strtoul])

dnl Flush all cached values in case something goes wrong
AC_CACHE_SAVE

# C++ compiler feature check start here
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
CH_CXX_BOOL
CH_CXX_EXCEPTIONS
CH_CXX_FOR_SCOPE
CH_CXX_NAMESPACE
CH_CXX_NAMESPACE_STD
CH_CXX_STDEXCEPT
CH_CXX_STL
CH_CXX_STREAM
CH_CXX_STRING

# Checking if C++ environment provides all required features which
# do not have workarounds.
ch_cxx_error_message=""
if test "${ch_cv_cxx_stream}" = no; then
    ch_cxx_error_message="${chimp_cxx_error_message}
${CXX} must have stream header files."
fi

if test "${ch_cv_cxx_string}" = no; then
    ch_cxx_error_message="${chimp_cxx_error_message}
${CXX} must have working string class."
fi

if test "${ch_cv_cxx_stl_headers}" = no; then
    ch_cxx_error_message="${chimp_cxx_error_message}
STL headers required, please see file STL."
fi

if test "${ch_cv_cxx_stl}" = no; then
    ch_cxx_error_message="${chimp_cxx_error_message}
${CXX} must support Standard Template Library (STL)."
fi

if test "${ch_cv_cxx_stdexcept}" = no; then
    ch_cxx_error_message="${chimp_cxx_error_message}
${CXX} must support standard exceptions, see file STL."
fi

AC_MSG_CHECKING(whether ${CXX} has all the required features)
if test -n "${ch_cxx_error_message}"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([${ch_cxx_error_message}])
else
    AC_MSG_RESULT(yes)
fi
AC_LANG_RESTORE

# Autoheader macros
AH_BOTTOM([/* If the C++ compiler we use doesn't have bool, then the following is
a near-perfect work-around.  You must make sure your code does not
depend on "int" and "bool" being two different types, in
overloading for instance. */
#ifndef CXX_HAVE_BOOL
typedef int bool;
#define true 1
#define false 0
#endif /* CXX_HAVE_BOOL */

/* Workaround for compilers with buggy for-loop scoping.
The trivial "if" statement provides the correct scoping to the for loop. */
#ifdef CXX_FOR_SCOPE_OLD
#define for if(1) for
#endif /* CXX_FOR_SCOPE_OLD */

/* If the C++ compiler doesn't support namespaces define these macros.
Use the macros rather than the explicit namespace syntax. */
#ifdef CXX_HAVE_NAMESPACE
#define CH_BEGIN_NAMESPACE namespace chimp {
#define CH_END_NAMESPACE }
#define CH_USING_NAMESPACE using namespace chimp
#define CH_CHIMP chimp
#else /* not CXX_HAVE_NAMESPACE */
#define CH_BEGIN_NAMESPACE
#define CH_END_NAMESPACE
#define CH_USING_NAMESPACE
#define CH_CHIMP
#endif /* not CXX_HAVE_NAMESPACE */

/* If the C++ compiler does not include system headers in namespace std
define these macros.  Use CH_STD:: rather than std::. */
#ifdef CXX_HAVE_NAMESPACE_STD
#define CH_STD std
#else /* not CXX_HAVE_NAMESPACE_STD */
#define CH_STD
#endif /* not CXX_HAVE_NAMESPACE_STD */])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/model/Makefile
                 test/Makefile
                 test/rtest.pl])
AC_OUTPUT
