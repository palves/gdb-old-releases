dnl written by Rob Savoye <rob@cygnus.com> for Cygnus Support
dnl CYGNUS LOCAL: This gets the right posix flag for gcc
AC_DEFUN(CY_TCL_LYNX_POSIX,
[AC_REQUIRE([AC_PROG_CC])AC_REQUIRE([AC_PROG_CPP])
AC_MSG_CHECKING([to see if this is LynxOS])
AC_CACHE_VAL(ac_cv_os_lynx,
[AC_EGREP_CPP(yes,
[/*
 * The old Lynx "cc" only defines "Lynx", but the newer one uses "__Lynx__"
 */
#if defined(__Lynx__) || defined(Lynx)
yes
#endif
], ac_cv_os_lynx=yes, ac_cv_os_lynx=no)])
#
if test "$ac_cv_os_lynx" = "yes" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(LYNX)
  AC_MSG_CHECKING([whether -mposix or -X is available])
  AC_CACHE_VAL(ac_cv_c_posix_flag,
  [AC_TRY_COMPILE(,[
  /*
   * This flag varies depending on how old the compiler is.
   * -X is for the old "cc" and "gcc" (based on 1.42).
   * -mposix is for the new gcc (at least 2.5.8).
   */
  #if defined(__GNUC__) && __GNUC__ >= 2
  choke me
  #endif
  ], ac_cv_c_posix_flag=" -mposix", ac_cv_c_posix_flag=" -X")])
  CC="$CC $ac_cv_c_posix_flag"
  AC_MSG_RESULT($ac_cv_c_posix_flag)
  else
  AC_MSG_RESULT(no)
fi
])
AC_DEFUN(CY_PATH_TKH, [
#
# Ok, lets find the tk source trees so we can use the headers
# If the directory (presumably symlink) named "tk" exists, use that one
# in preference to any others.  Same logic is used when choosing library
# and again with Tcl.
# Note the gross little conversion here of srcdir by cd'ing to the found
# directory. This converts the path from a relative to an absolute, so
# recursive cache variables for the path will work right.
#
AC_MSG_CHECKING(for Tk headers)
installed=0
no_tk=true
AC_CACHE_VAL(ac_cv_c_tkh,
[for i in `ls -d ${srcdir}/../tk* ${srcdir}/../../tk* ${srcdir}/../../../tk* 2>/dev/null` ; do
  if test -f $i/tk.h ; then
    installed=1
    rootme=`pwd`
    cd $i
    ac_cv_c_tkh=`pwd`
    cd $rootme
  fi
done
# if we can't find it, see if one is installed
if test x"$ac_cv_c_tkh" = x ; then
  if test -f $prefix/include/tk.h; then
    installed=1
    ac_cv_c_tkh="$prefix/include"
    rootme=`pwd`
    cd $i
    ac_cv_c_tkh=`pwd`
    cd $rootme
  else
    AC_HEADER_CHECK(tk.h, installed=1 ac_cv_c_tkh=installed)
  fi
fi
])
if test x"$ac_cv_c_tkh" = x"installed" ; then
  AC_MSG_RESULT([is installed])
  no_tk=
else
  if test x"$ac_cv_c_tkh" != x ; then
    AC_MSG_RESULT([found in $ac_cv_c_tkh])
    # this hack is cause the TKHDIR won't print if there is a "-I" in it.
    TKHDIR="-I${ac_cv_c_tkh}"
    no_tk=
  fi
  if test $installed -eq 0 -a x"${ac_cv_c_tkh}" = x ; then
    TKHDIR="# no Tk directory found"
    AC_MSG_WARN(Can't find Tk headers)
  fi
fi
AC_SUBST(TKHDIR)])

AC_DEFUN(CY_PATH_TKLIB, [
#
# Ok, lets find the tk library
# First, look for one uninstalled.  
#
if test x"$no_tk" = x ; then
  AC_MSG_CHECKING(for Tk library)
  AC_CACHE_VAL(ac_cv_c_tklib,
  [for i in `ls -d ../tk* ../../tk* ../../../tk* 2>/dev/null` ; do
    if test -f "$i/Makefile" ; then
      rootme=`pwd`
      cd $i
      ac_cv_c_tklib=`pwd`/libtk.a
      cd $rootme
      fi
  done
  dnl If not found, look for installed version
  dnl This doesn't work because of unresolved symbols.
  dnl AC_HAVE_LIBRARY(libtk.a, installed=1, installed=0)
  if test x"$TKLIB" = x ; then
    if test -f $prefix/lib/libtk.a; then
      ac_cv_c_tklib="-ltk"
    fi
  fi
  # If still not found, assume Tk simply hasn't been built yet
  if test x"$ac_cv_c_tklib" = x ; then
    for i in `ls -d ../tk* ./../tk* ./../../tk* 2>/dev/null` ; do
      if test -f "$i/tk.h" ; then
        ac_cv_c_tklib=$i/libtk.a
      fi
    done    
  fi
  ])
  if test x"$ac_cv_c_tklib" = x ; then
    TKLIB="# no Tk library found"
    AC_MSG_WARN(Can't find Tk library)
  else
    TKLIB=$ac_cv_c_tklib
    AC_MSG_RESULT(found $TKLIB)
    no_tk=
  fi
else
   TKLIB="# no Tk headers found"
fi
AC_SUBST(TKLIB)
])
AC_DEFUN(CY_PATH_TCLH, [
#
# Ok, lets find the tcl source trees so we can use the headers
# If the directory (presumably symlink) named "tcl" exists, use that one
# in preference to any others.
#
AC_MSG_CHECKING(for Tcl headers)
no_tcl=true
installed=0
AC_CACHE_VAL(ac_cv_c_tclh,
[for i in `ls -d ${srcdir}/../tcl* ${srcdir}/../../tcl* ${srcdir}/../../../tcl* 2>/dev/null` ; do
  if test -f $i/tcl.h ; then
    installed=1
    rootme=`pwd`
    cd $i
    ac_cv_c_tclh=`pwd`
    cd $rootme
  fi
done
# if we can't find it, see if one is installed
if test x"$ac_cv_c_tclh" = x ; then
  if test -f $prefix/include/tclInt.h; then
    installed=1
    rootme=`pwd`
    cd $i
    ac_cv_c_tclh=`pwd`
    cd $rootme
  else
    AC_HEADER_CHECK(tcl.h, installed=1 ac_cv_c_tclh=installed)
  fi
fi
])
if test x"${ac_cv_c_tkh}" = x"installed" ; then
  AC_MSG_RESULT([is installed])
  no_tcl=
else
  if test $installed -eq 0 -a x"${ac_cv_c_tclh}" = x ; then
    TCLHDIR="# no Tcl headers found"
    AC_MSG_WARN(Can't find Tcl headers)
  fi
  if test x"${ac_cv_c_tclh}" != x ; then
    AC_MSG_RESULT([found in $ac_cv_c_tclh])
    # this hack is cause the TCLHDIR won't print if there is a "-I" in it.
    TCLHDIR="-I${ac_cv_c_tclh}"
    no_tcl=
  fi
fi
AC_SUBST(TCLHDIR)])

AC_DEFUN(CY_PATH_TCLLIB, [
#
# Ok, lets find the tcl library
# First, look for one uninstalled.  
#
if test x"$no_tcl" = x ; then
  AC_MSG_CHECKING(for Tcl library)
  AC_CACHE_VAL(ac_cv_c_tcllib,
  [for i in `ls -d ../tcl* ../../tcl* ../../../tcl* 2>/dev/null` ; do
    if test -f "$i/Makefile" ; then
      rootme=`pwd`
      cd $i
      ac_cv_c_tcllib=`pwd`/libtcl.a
      cd $rootme
  fi
  done
  dnl If not found, look for installed version
  dnl This doesn't work because of unresolved symbols.
  dnl AC_HAVE_LIBRARY(libtcl.a, installed=1, installed=0)
  if test x"$ac_cv_c_tcllib" = x ; then
    if test -f $prefix/lib/libtcl.a; then
      ac_cv_c_tcllib="-ltcl"
    fi
  fi
  # If still not found, assume Tcl simply hasn't been built yet
  if test x"$ac_cv_c_tcllib" = x ; then
    for i in `ls -d ../tcl* ../../tcl* ../../../tcl* 2>/dev/null` ; do
      if test -f "$i/tcl.h" ; then
        rootme=`pwd`
        cd $i
        ac_cv_c_tcllib=`pwd`/libtcl.a
        cd $rootme
      fi
    done    
  fi
  ])
  if test x"$ac_cv_c_tcllib" = x ; then
    TCLLIB="# no Tcl library found"
    AC_MSG_WARN(Can't find Tcl library)
  else
    TCLLIB=$ac_cv_c_tcllib
    AC_MSG_RESULT(found $TCLLIB)
    no_tcl=
  fi
fi
AC_SUBST(TCLLIB)
])
AC_DEFUN(CY_PATH_TK, [
  CY_PATH_TKH
  CY_PATH_TKLIB
])
AC_DEFUN(CY_PATH_TCL, [
  CY_PATH_TCLH
  CY_PATH_TCLLIB
])

