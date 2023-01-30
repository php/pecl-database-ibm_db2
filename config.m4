PHP_ARG_WITH(IBM_DB2, for IBM_DB2 support,
[  --with-IBM_DB2=[DIR]      Include IBM DB2 Universal Database and Cloudscape support.
                          DIR is the location of the DB2 application development 
                          headers and libraries. Set the PHP_IBM_DB2_LIB
                          environment variable to set the specific location of
                          the DB2 libraries])

if test "$PHP_IBM_DB2" != "no"; then
  dnl # --with-IBM_DB2 -> check with-path  	 
  SEARCH_PATH="$PHP_IBM_DB2_LIB $SEARCH_PATH $PHP_IBM_DB2 $IBM_DB_HOME $DB2PATH $DB2DIR"

  dnl Scan the library path for LUW, clidriver, and libdb400 in the usual
  dnl places, also assuming include/ is in the directory too.
  for i in $SEARCH_PATH ; do
    dnl XXX: The messages kinda suck and don't indicate which path
    dnl (combined with AC_MSG_* spew from AC_CHECK_LIB)
    dnl XXX: Macros for this? Can these be merged?

    dnl LUW ships its client libraries in lib64/32 (at least on amd64 linux)
    AC_CHECK_SIZEOF([long])
    AC_MSG_CHECKING([if we're on a 64-bit platform])
    AS_IF([test "$ac_cv_sizeof_long" -eq 4],[
      AC_MSG_RESULT([no])
      PHP_CHECK_LIBRARY(db2, SQLDriverConnect, [
        PHP_ADD_LIBPATH($i/lib32, IBM_DB2_SHARED_LIBADD)
        PHP_ADD_LIBRARY(db2, 1, IBM_DB2_SHARED_LIBADD)
        PHP_ADD_INCLUDE($i/include)
        break
      ], [], "-L$i/lib32" )
    ],[
      AC_MSG_RESULT([yes])
      PHP_CHECK_LIBRARY(db2, SQLDriverConnect, [
        PHP_ADD_LIBPATH($i/lib64, IBM_DB2_SHARED_LIBADD)
        PHP_ADD_LIBRARY(db2, 1, IBM_DB2_SHARED_LIBADD)
        PHP_ADD_INCLUDE($i/include)
        break
      ], [], "-L$i/lib64" )
    ])
    dnl The standalone clidriver package uses lib/
    PHP_CHECK_LIBRARY(db2, SQLDriverConnect, [
      PHP_ADD_LIBPATH($i/lib, IBM_DB2_SHARED_LIBADD)
      PHP_ADD_LIBRARY(db2, 1, IBM_DB2_SHARED_LIBADD)
      PHP_ADD_INCLUDE($i/include)
      break
    ], [
    ], "-L$i/lib" )
    dnl Special cases for PASE
    dnl SG ships a custom libdb400 (with renamed funcs to co-exist w/ ODBC)
    dnl it requires some special handling for headers too
    PHP_CHECK_LIBRARY(db400sg, LDBDriverConnect, [
      IBM_DB2_PASE=yes
      dnl from RPMs libdb400sg-devel and sqlcli-devel
      dnl as IBM i doesn't ship SQL/CLI headers w/ PASE (and RPM's in subdir)
      PHP_ADD_LIBPATH($i/lib, IBM_DB2_SHARED_LIBADD)
      PHP_ADD_LIBRARY(db400sg, 1, IBM_DB2_SHARED_LIBADD)
      PHP_ADD_INCLUDE(/QOpenSys/pkgs/include/cli-sg)
      PHP_ADD_INCLUDE(/QOpenSys/pkgs/include/cli)
      break
    ], [
    ], "-L$i/lib" )
    dnl Probably vanilla libdb400
    dnl XXX: For PASE, libdb400 is likely on the default path
    PHP_CHECK_LIBRARY(db400, SQLDriverConnect, [
      IBM_DB2_PASE=yes
      dnl from RPM sqlcli-devel
      PHP_ADD_LIBPATH($i/lib, IBM_DB2_SHARED_LIBADD)
      PHP_ADD_LIBRARY(db400, 1, IBM_DB2_SHARED_LIBADD)
      PHP_ADD_INCLUDE(/QOpenSys/pkgs/include/cli)
      break
    ], [
    ], "-L$i/lib" )
  done

  if test "$IBM_DB2_PASE" = "yes" ; then
    PHP_NEW_EXTENSION(ibm_db2, ibm_db2.c, $ext_shared,,-DPASE)
  else
    PHP_NEW_EXTENSION(ibm_db2, ibm_db2.c, $ext_shared)
  fi

  PHP_SUBST(IBM_DB2_SHARED_LIBADD)
fi
