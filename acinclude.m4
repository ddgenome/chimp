dnl
dnl @synopsis CH_CXX_BOOL
dnl
dnl This macro checks if the C++ compiler supports bool.
dnl
dnl This is a modified version of a macro distributed with autotools-0.10
dnl which was written by Eleftherios Gkioulekas <lf@amath.washington.edu>.
dnl
dnl If bool is supported it defines CXX_HAVE_BOOL
dnl
dnl Put something like this in your acconfig.h:
dnl   #undef CXX_HAVE_BOOL
dnl   #ifndef CXX_HAVE_BOOL
dnl   typedef int bool;
dnl   #define true 1
dnl   #define false 0
dnl   #endif /* not CXX_HAVE_BOOL */
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_BOOL],
[
AC_REQUIRE([AC_PROG_CXX])
AC_CACHE_CHECK([whether ${CXX} has builtin bool],
ch_cv_cxx_bool,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_LINK([
int f(int i) { return i; }
int f(bool b) { return b; }
],[
  bool b1 = true;
  bool b2 = false;
  return f(b1);
],
ch_cv_cxx_bool=yes,
ch_cv_cxx_bool=no)
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_bool}" = yes; then
AC_DEFINE(CXX_HAVE_BOOL, 1, [Define if C++ has working bool])
fi
])dnl
dnl
dnl @synopsis CH_CXX_EXCEPTIONS
dnl
dnl This macro checks is the C++ environment supports exceptions and
dnl handles them properly.
dnl
dnl It defines CXX_HAVE_EXCEPTIONS if exceptions are supported.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_EXCEPTIONS],
[
AC_REQUIRE([AC_PROG_CXX])
AC_CACHE_CHECK([whether ${CXX} supports exceptions],
ch_cv_cxx_exceptions,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_RUN(
[
#include <stdlib.h>
int main() {
  try { throw 0; }
  catch (int i) { exit(i); }
  exit(1);
}
],
ch_cv_cxx_exceptions=yes,
ch_cv_cxx_exceptions=no,
[
dnl for cross-compilers
AC_TRY_COMPILE(,
[
  try { throw 0; }
  catch (int i) { return i; }
  return 1;
],
ch_cv_cxx_exceptions=yes,
ch_cv_cxx_exceptions=no)
])
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_exceptions}" = yes; then
AC_DEFINE(CXX_HAVE_EXCEPTIONS, 1, [Define if C++ has exceptions])
fi
])dnl
dnl
dnl @synopsis CH_CXX_FOR_SCOPE
dnl
dnl This macro checks if the scoping of C++ for loop variables is correct.
dnl This is a modified version of a macro distributed with autotools-0.10
dnl which was written by Eleftherios Gkioulekas <lf@amath.washington.edu>.
dnl
dnl If scoping is incorrect it defines CXX_FOR_SCOPE_OLD
dnl
dnl Put something like this in your acconfig.h, the trivial if gives the
dnl proper behavior:
dnl   #undef CXX_FOR_SCOPE_OLD
dnl   #ifdef CXX_FOR_SCOPE_OLD
dnl   #define for if(1) for
dnl   #endif /* CXX_FOR_SCOPE_OLD */
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_FOR_SCOPE],
[
AC_REQUIRE([AC_PROG_CXX])
AC_CACHE_CHECK([whether ${CXX} for loop has proper variable scoping],
ch_cv_cxx_for_scope,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE(,
[
  for (int i = 0; i < 10; i++) {}
  for (int i = 0; i < 10; i++) {}
],
ch_cv_cxx_for_scope=yes,
ch_cv_cxx_for_scope=no)
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_for_scope}" = no; then
AC_DEFINE(CXX_FOR_SCOPE_OLD, 1, [Define if C++ has old for scope])
fi
])dnl
dnl
dnl @synopsis CH_CXX_NAMESPACE
dnl
dnl Check if the C++ compiler supports namespaces.
dnl
dnl If namespaces are supported it defines CXX_HAVE_NAMESPACE
dnl
dnl Put something like this in your acconfig.h:
dnl   #ifdef CXX_HAVE_NAMESPACE
dnl   #define MY_BEGIN_NAMESPACE namespace my {
dnl   #define MY_END_NAMESPACE }
dnl   #define MY_USING_NAMESPACE using namespace my
dnl   #define MY_MY my
dnl   #else /* not CXX_HAVE_NAMESPACE */
dnl   #define MY_BEGIN_NAMESPACE
dnl   #define MY_END_NAMESPACE
dnl   #define MY_USING_NAMESPACE
dnl   #define MY_MY
dnl   #endif /* not CXX_HAVE_NAMESPACE */
dnl then use, e. g., MY_BEGIN_NAMESPACE rather than ``namespace my {''.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_NAMESPACE],
[
AC_REQUIRE([AC_PROG_CXX])
AC_CACHE_CHECK([whether ${CXX} supports namespace],
ch_cv_cxx_namespace,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_LINK(
[
namespace foo {
  int i = 0;
  int f();
}
int foo::f() { return foo::i; }
],[
  foo::i = foo::f();
  using namespace foo;
  i = f();
],
ch_cv_cxx_namespace=yes,
ch_cv_cxx_namespace=no)
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_namespace}" = yes; then
AC_DEFINE(CXX_HAVE_NAMESPACE, 1, [Define if C++ has namespaces])
fi
])dnl
dnl
dnl @synopsis CH_CXX_NAMESPACE_STD
dnl
dnl This macro checks if the C++ compiler puts standard library
dnl headers in the namespace std.
dnl
dnl If std namespace is supported it defines CXX_HAVE_NAMESPACE_STD.
dnl Put something like this in your acconfig.h:
dnl   #ifdef CXX_HAVE_NAMESPACE_STD
dnl   #define MY_STD std
dnl   #else /* not CXX_HAVE_NAMESPACE_STD */
dnl   #define MY_STD
dnl   #endif /* not CXX_HAVE_NAMESPACE_STD */
dnl then use MY_STD:: rather than std:: in your code.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_NAMESPACE_STD],
[
dnl only check if compiler supports namespaces
AC_REQUIRE([CH_CXX_NAMESPACE])
if test "${ch_cv_cxx_namespace}" = yes; then
AC_CACHE_CHECK([whether ${CXX} uses the std namespace],
ch_cv_cxx_namespace_std,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_LINK(
[
#include <iostream>
#include <string>
],[
  std::string s("foo");
  std::cout << s << std::endl;
],
ch_cv_cxx_namespace_std=yes,
ch_cv_cxx_namespace_std=no)
AC_LANG_RESTORE
])
else
    ch_cv_cxx_namespace_std=no
fi
if test "${ch_cv_cxx_namespace_std}" = yes; then
AC_DEFINE(CXX_HAVE_NAMESPACE_STD, 1, [Define if C++ uses the std namespace])
fi
])dnl
dnl
dnl @synopsis CH_CXX_STDEXCEPT
dnl
dnl This macro checks is the C++ environment supports standard exceptions.
dnl
dnl It defines CXX_HAVE_STDEXCEPT if they are available.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_STDEXCEPT],
[
AC_REQUIRE([CH_CXX_EXCEPTIONS])
AC_CACHE_CHECK([whether ${CXX} has working standard exceptions],
ch_cv_cxx_stdexcept,
[
if test "${ch_cv_cxx_exceptions}" = no; then
    ch_cv_cxx_stdexcept=no
else
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_CPP(
[
#include <stdexcept>
],
ch_cv_cxx_stdexcept=yes,
ch_cv_cxx_stdexcept=no)
AC_LANG_RESTORE
fi
])
if test "${ch_cv_cxx_stdexcept}" = yes; then
AC_DEFINE(CXX_HAVE_STDEXCEPT, 1, [Define if C++ has standard exceptions])
fi
])dnl
dnl
dnl @synopsis CH_CXX_STL
dnl
dnl Check if the C++ environment has STL headers.  This macro checks
dnl for the presence of some standard STL header files and then tries
dnl to compile a short program with one of them.
dnl
dnl If everything goes well, it defines CXX_HAVE_STL
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl 
AC_DEFUN([CH_CXX_STL],
[
AC_REQUIRE([CH_CXX_NAMESPACE_STD])
AC_CACHE_CHECK([whether C++ environment has STL headers],
ch_cv_cxx_stl_headers,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_CPP(
[
#include <deque>
#include <map>
#include <list>
#include <queue>
#include <set>
#include <vector>
],
ch_cv_cxx_stl_headers=yes,
ch_cv_cxx_stl_headers=no)
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_stl_headers}" = no; then
AC_MSG_WARN(STL header files not found.  Try setting the CXXFLAGS environment variable.)
else
AC_CACHE_CHECK([whether ${CXX} can compile a simple STL program],
ch_cv_cxx_stl,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE(
[
#ifdef CXX_HAVE_NAMESPACE_STD
#define CH_STD std
#else
#define CH_STD
#endif
#include <vector>
],[
  CH_STD::vector<int> vi;
  for (int i = 0; i < 10; i++)
    vi.push_back(i)
],
ch_cv_cxx_stl=yes,
ch_cv_cxx_stl=no)
AC_LANG_RESTORE
])
    if test "${ch_cv_cxx_stl}" = no; then
AC_MSG_WARN(Your C++ compiler STL support is broken.)
    fi
fi
])dnl
dnl
dnl @synopsis CH_CXX_STREAM
dnl
dnl This macros makes sure the C++ compiler has stream support and
dnl checks if <sstream> or <strstream> is supported.
dnl
dnl If <sstream> is found it defines HAVE_SSTREAM, otherwise if
dnl <strstream> is found it defines HAVE_STRSTREAM.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_STREAM],
[
AC_REQUIRE([AC_PROG_CXXCPP])
AC_CACHE_CHECK([whether ${CXX} has iostream and fstream support],
ch_cv_cxx_stream,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_CPP([
#include <iostream>
#include <fstream>
],
ch_cv_cxx_stream=yes,
ch_cv_cxx_stream=no)
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_stream}" = yes; then
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
    if test "${ch_cv_cxx_header_style}" = old; then
AC_CHECK_HEADERS(sstream.h strstream.h,[
ch_cv_cxx_stream=yes
break
],
ch_cv_cxx_stream=no)
    else
AC_CHECK_HEADERS(sstream strstream,[
ch_cv_cxx_stream=yes
break
],
ch_cv_cxx_stream=no)
    fi
AC_LANG_RESTORE
fi
])dnl
dnl
dnl @synopsis CH_CXX_STRING
dnl
dnl This macro checks if the C++ compiler has a working string class.
dnl
dnl @version $Id: acinclude.m4,v 1.1.1.1 2004/11/25 20:24:04 banjo Exp $
dnl @author David J. Dooling <banjo@users.sourceforge.net>
dnl
AC_DEFUN([CH_CXX_STRING],
[
AC_REQUIRE([CH_CXX_NAMESPACE_STD])
AC_CACHE_CHECK([whether ${CXX} has working string class],
ch_cv_cxx_string,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_LINK(
[
#ifdef CXX_HAVE_NAMESPACE_STD
#define CH_STD std
#else
#define CH_STD
#endif
#include <string>
],[
  CH_STD::string s1("test");
  CH_STD::string s2("worked");
  s1.size();
  s2.c_str();
  CH_STD::string s12 = s1 + " " + s2;
],
ch_cv_cxx_string=yes,
ch_cv_cxx_string=no)
AC_LANG_RESTORE
])
if test "${ch_cv_cxx_string}" = no; then
AC_MSG_WARN(Your C++ compiler string support does not work.)
fi
])dnl
