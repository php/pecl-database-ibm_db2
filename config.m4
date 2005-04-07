dnl $Id$
dnl config.m4 for extension ibm_db2

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(IBM_DB2, for IBM_DB2 support,
[  --with-IBM_DB2[=DIR]             Include IBM DB2 Univeral Database and Cloudscape support.
                                 DIR is the location of the DB2 application development
                                 headers and libraries, defaults to /opt/IBM/db2/V8.1])
dnl PHP_ARG_ENABLE(IBM_DB2, whether to enable IBM_DB2 support,
dnl Make sure that the comment is aligned:
dnl [  --enable-IBM_DB2           Enable IBM_DB2 support])

if test "$PHP_IBM_DB2" != "no"; then

  dnl # --with-IBM_DB2 -> check with-path
  SEARCH_PATH="$DB2PATH $DB2DIR /opt/IBM/db2/V8.1/ /home/db2inst1/sqllib /usr/local /usr"

  if test -r $PHP_IBM_DB2/; then
     AC_MSG_CHECKING([for DB2 CLI files in $PHP_IBM_DB2])
     if test -r $PHP_IBM_DB2/lib/libdb2.so || test -r $PHP_IBM_DB2/lib/libdb2.a ; then
       if test -r "$PHP_IBM_DB2/include/sqlcli1.h" ; then
	 IBM_DB2_DIR=$PHP_IBM_DB2
	 AC_MSG_RESULT(yes)
       fi
     fi
   else
     AC_MSG_CHECKING([for DB2 CLI files in default path])
     for i in $SEARCH_PATH ; do
       if test -r $i/lib/libdb2.so || test -r $i/lib/libdb2.a ; then
	 if test -r "$i/include/sqlcli1.h" ; then
	   IBM_DB2_DIR=$i
	   AC_MSG_RESULT(found in $i)
	   break
	 fi
       fi
     done
   fi

   if test -z "$IBM_DB2_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([Please reinstall the DB2 CLI distribution])
   fi

  dnl # --with-IBM_DB2 -> add include path
  PHP_ADD_INCLUDE($IBM_DB2_DIR/include)


  dnl # --with-IBM_DB2 -> check for lib and symbol presence
  LIBNAME=db2
  LIBSYMBOL=SQLConnect

dnl #  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
dnl #  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $IBM_DB2_DIR/lib, IBM_DB2_SHARED_LIBADD)
    AC_DEFINE(HAVE_DB2CLILIB,1,[ ])
dnl #  ],[
dnl #    AC_MSG_ERROR([wrong DB2 CLI lib version or lib not found])
dnl #  ],[
dnl #    -L$IBM_DB2_DIR/lib -lm -ldl
dnl #  ])

  PHP_NEW_EXTENSION(ibm_db2, ibm_db2.c, $ext_shared)

  PHP_SUBST(IBM_DB2_SHARED_LIBADD)
fi
