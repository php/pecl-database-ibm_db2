#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#else

#ifdef _WIN64
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;
#else
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#endif
#endif

/*
 * Added in IBM i 7.5, also in LUW 11.1 MP1/FP1
 * see: https://www.ibm.com/docs/en/db2/11.1?topic=database-mod-pack-fix-pack-updates#c0061179__FP1
 */
#ifndef SQL_BOOLEAN
#define SQL_BOOLEAN 16
#endif

/* Needed for backward compatibility (SQL_ATTR_DBC_SYS_NAMING not defined prior to DB2 10.1.0.2) */
#ifndef SQL_ATTR_DBC_SYS_NAMING
#define SQL_ATTR_DBC_SYS_NAMING 3017
#endif

/* needed for backward compatibility (SQL_XML not defined prior to DB2 v9) */
#ifndef SQL_XML
#define SQL_XML -370
#endif

/* needed for backward compatibility (SQL_ATTR_ROWCOUNT_PREFETCH not defined prior to DB2 9.5.0.3) */
#ifndef SQL_ATTR_ROWCOUNT_PREFETCH
#define SQL_ATTR_ROWCOUNT_PREFETCH 2592
#define SQL_ROWCOUNT_PREFETCH_OFF   0
#define SQL_ROWCOUNT_PREFETCH_ON    1
#endif

/* SQL_ATTR_USE_TRUSTED_CONTEXT, 
 * SQL_ATTR_TRUSTED_CONTEXT_USERID and 
 * SQL_ATTR_TRUSTED_CONTEXT_PASSWORD
 * not defined prior to DB2 v9 */
#ifndef SQL_ATTR_USE_TRUSTED_CONTEXT
#define SQL_ATTR_USE_TRUSTED_CONTEXT 2561
#define SQL_ATTR_TRUSTED_CONTEXT_USERID 2562
#define SQL_ATTR_TRUSTED_CONTEXT_PASSWORD 2563
#endif

/* Needed for Backward compatibility */
#ifndef SQL_DECFLOAT
#define SQL_DECFLOAT -360
#endif

/* This is used in db2_last_insert_id.
 * We allocate a buffer of size 32 as per 
 * recommendations from the CLI IDS team */
#define MAX_IDENTITY_DIGITS 32

/* Used in _php_parse_options */
#define DB2_ERRMSG 1
#define DB2_ERR 2

/* DB2 instance environment variable */
#define DB2_VAR_INSTANCE "DB2INSTANCE="

/* Visual name for the connection */
#define DB2_CONN_NAME "DB2 Connection"

/* Visual name for the persistent connection */
#define DB2_PCONN_NAME "DB2 Persistent Connection"

/* Visual name for the statement */
#define DB2_STMT_NAME "DB2 Statement"

/******** Makes code compatible with the options used by the user */
#define DB2_BINARY 1
#define DB2_CONVERT 2
#define DB2_PASSTHRU 3

/* maximum sizes */
#define USERID_LEN 16
#define ACCTSTR_LEN 200
#define APPLNAME_LEN 32
#define WRKSTNNAME_LEN 18

/* 1.9.7 - LUW DB2 Connect 10.5 missing SQL_UTF8_CHAR, set to fake/unused ordinal */
#ifndef SQL_UTF8_CHAR
#define SQL_UTF8_CHAR -334
#endif

#ifdef PASE
/* IBM i generically changed  (remove ifdef PASE) */
#define SQL_IS_INTEGER 0
#define SQL_IS_UINTEGER	0 
#define SQL_BEST_ROWID 0
#define SQLLEN long
#define SQLFLOAT double
#define SQLUINTEGER SQLINTEGER
#define SQLUSMALLINT SQLSMALLINT
/* IBM i long is same ordinal, set to fake/unused ordinal (remove ifdef PASE) */
#undef SQL_LONGVARCHAR
#define SQL_LONGVARCHAR -334 
#undef SQL_LONGVARGRAPHIC
#define SQL_LONGVARGRAPHIC -335
#undef SQL_LONGVARBINARY
#define SQL_LONGVARBINARY -336
#undef SQL_WLONGVARCHAR
#define SQL_WLONGVARCHAR -337
/* IBM i support V6R1+, ignore V5R4- (remove ifdef PASE) */
#undef SQL_BINARY
#define  SQL_BINARY          -2
#undef SQL_VARBINARY
#define  SQL_VARBINARY       -3
#undef SQL_C_BINARY
#define  SQL_C_BINARY	SQL_BINARY
/* IBM i mv from ibm_db2.c to php_ibm_db2.h  (correct) */
#define SQL_ATTR_INFO_USERID         10103
#define SQL_ATTR_INFO_WRKSTNNAME     10104
#define SQL_ATTR_INFO_APPLNAME       10105
#define SQL_ATTR_INFO_ACCTSTR        10106
#define SQL_ATTR_QUERY_TIMEOUT		SQL_QUERY_TIMEOUT
/* orig  - IBM i SQL_ATTR_JOB_SORT_SEQUENCE (customer request DB2 PTF) */
#define SQL_ATTR_CONN_SORT_SEQUENCE  10046
#define SQL_HEX_SORT_SEQUENCE 0                   
#define SQL_JOB_SORT_SEQUENCE 1                   
#define SQL_JOBRUN_SORT_SEQUENCE 2
/* 1.9.7 - IBM i consultant request switch subsystem QSQSRVR job (customer workload issues) */                 
#ifndef SQL_ATTR_SERVERMODE_SUBSYSTEM
#define SQL_ATTR_SERVERMODE_SUBSYSTEM 10204
#endif
/* 1.9.7 - IBM i monitor switch user profile applications (customer security issue) */
#define DB2_IBM_I_PROFILE_UID_MAX 10

/* 1.9.7 - IBM i force UTF-8 CCSID (DBCS customer issue) */
extern int SQLOverrideCCSID400(int newCCSID);

#endif /* PASE */

#ifndef PASE
#define DB2_SCROLLABLE SQL_CURSOR_KEYSET_DRIVEN
#define DB2_FORWARD_ONLY SQL_SCROLL_FORWARD_ONLY
#else
#define DB2_SCROLLABLE SQL_CURSOR_DYNAMIC
#define DB2_FORWARD_ONLY SQL_CURSOR_FORWARD_ONLY
#define SQL_SCROLL_FORWARD_ONLY SQL_CURSOR_FORWARD_ONLY
#endif

/* CB 20200826 - This is documented, but not defined by headers in 7.2/7.4 */
#ifndef SQL_ATTR_NON_HEXCCSID
#define SQL_ATTR_NON_HEXCCSID 10203
#endif

/* 1.9.7 - IBM i + LUW 10.5 system naming on (*libl)/file.mbr */
#define DB2_I5_NAMING_ON  SQL_TRUE
#define DB2_I5_NAMING_OFF SQL_FALSE

/* 1.9.7 - IBM i + LUW 10.5 new IBM i attributes */
#ifndef SQL_ATTR_DECIMAL_SEP
#define SQL_ATTR_DATE_FMT                          3025
#define SQL_ATTR_DATE_SEP                          3026 
#define SQL_ATTR_TIME_FMT                          3027
#define SQL_ATTR_TIME_SEP                          3028
#define SQL_ATTR_DECIMAL_SEP                       3029
#endif

/* 1.9.7 - LUW to IBM i need isolation mode *NONE (required non journal CRTLIB) */
#ifndef SQL_TXN_NO_COMMIT
#define SQL_TXN_NO_COMMIT SQL_TXN_NOCOMMIT
#endif
#define DB2_I5_TXN_NO_COMMIT SQL_TXN_NO_COMMIT
#define DB2_I5_TXN_READ_UNCOMMITTED SQL_TXN_READ_UNCOMMITTED
#define DB2_I5_TXN_READ_COMMITTED SQL_TXN_READ_COMMITTED
#define DB2_I5_TXN_REPEATABLE_READ SQL_TXN_REPEATABLE_READ
#define DB2_I5_TXN_SERIALIZABLE SQL_TXN_SERIALIZABLE

/* 1.9.7 - PASE change to LUW IBMi style (reserve non-IBMi other for LUW) */
#ifdef PASE
#define SQL_IBMi_FMT_ISO SQL_FMT_ISO
#define SQL_IBMi_FMT_USA SQL_FMT_USA
#define SQL_IBMi_FMT_EUR SQL_FMT_EUR
#define SQL_IBMi_FMT_JIS SQL_FMT_JIS
#define SQL_IBMi_FMT_MDY SQL_FMT_MDY
#define SQL_IBMi_FMT_DMY SQL_FMT_DMY
#define SQL_IBMi_FMT_YMD SQL_FMT_YMD
#define SQL_IBMi_FMT_JUL SQL_FMT_JUL
#define SQL_IBMi_FMT_HMS SQL_FMT_HMS
#define SQL_IBMi_FMT_JOB SQL_FMT_JOB
#endif /* PASE */

/* 1.9.7 - LUW to IBM i needed for backward compatibility (not defined prior to DB2 10.5) */
#ifndef SQL_IBMi_FMT_ISO
#define SQL_IBMi_FMT_ISO                  1
#define SQL_IBMi_FMT_USA                  2
#define SQL_IBMi_FMT_EUR                  3
#define SQL_IBMi_FMT_JIS                  4
#define SQL_IBMi_FMT_MDY                  5
#define SQL_IBMi_FMT_DMY                  6
#define SQL_IBMi_FMT_YMD                  7
#define SQL_IBMi_FMT_JUL                  8
#define SQL_IBMi_FMT_HMS                  9
#define SQL_IBMi_FMT_JOB                  10
#define SQL_SEP_SLASH                     1
#define SQL_SEP_DASH                      2
#define SQL_SEP_PERIOD                    3
#define SQL_SEP_COMMA                     4
#define SQL_SEP_BLANK                     5
#define SQL_SEP_COLON                     6
#define SQL_SEP_JOB                       7
#endif

/* 1.9.7 - LUW to IBM i new option attributes using IBMi_ (not defined prior to DB2 10.5) */
#define DB2_I5_FMT_ISO SQL_IBMi_FMT_ISO
#define DB2_I5_FMT_USA SQL_IBMi_FMT_USA
#define DB2_I5_FMT_EUR SQL_IBMi_FMT_EUR
#define DB2_I5_FMT_JIS SQL_IBMi_FMT_JIS
#define DB2_I5_FMT_MDY SQL_IBMi_FMT_MDY
#define DB2_I5_FMT_DMY SQL_IBMi_FMT_DMY
#define DB2_I5_FMT_YMD SQL_IBMi_FMT_YMD
#define DB2_I5_FMT_JUL SQL_IBMi_FMT_JUL
#define DB2_I5_FMT_JOB SQL_IBMi_FMT_JOB
#define DB2_I5_FMT_HMS SQL_IBMi_FMT_HMS
/* 1.9.7 - LUW to IBM i new option attributes match (not defined prior to DB2 10.5) */
#define DB2_I5_SEP_SLASH SQL_SEP_SLASH
#define DB2_I5_SEP_DASH SQL_SEP_DASH
#define DB2_I5_SEP_PERIOD SQL_SEP_PERIOD
#define DB2_I5_SEP_COMMA SQL_SEP_COMMA
#define DB2_I5_SEP_BLANK SQL_SEP_BLANK
#define DB2_I5_SEP_COLON SQL_SEP_COLON
#define DB2_I5_SEP_JOB SQL_SEP_JOB

#ifdef PASE
#define DB2_I5_FETCH_ON SQL_TRUE
#define DB2_I5_FETCH_OFF SQL_FALSE
#define DB2_I5_JOB_SORT_ON  SQL_TRUE
#define DB2_I5_JOB_SORT_OFF SQL_FALSE
#define DB2_I5_DBCS_ALLOC_ON  SQL_TRUE
#define DB2_I5_DBCS_ALLOC_OFF SQL_FALSE
#define DB2_I5_CHAR_TRIM_ON  SQL_TRUE
#define DB2_I5_CHAR_TRIM_OFF SQL_FALSE
#define DB2_FIRST_IO SQL_FIRST_IO
#define DB2_ALL_IO SQL_ALL_IO
#endif

#define DB2_AUTOCOMMIT_ON SQL_AUTOCOMMIT_ON
#define DB2_AUTOCOMMIT_OFF SQL_AUTOCOMMIT_OFF

#define DB2_ROWCOUNT_PREFETCH_OFF SQL_ROWCOUNT_PREFETCH_OFF
#define DB2_ROWCOUNT_PREFETCH_ON SQL_ROWCOUNT_PREFETCH_ON

#ifndef PASE
#define DB2_DEFERRED_PREPARE_ON SQL_DEFERRED_PREPARE_ON
#define DB2_DEFERRED_PREPARE_OFF SQL_DEFERRED_PREPARE_OFF
#endif

#define DB2_PARAM_IN SQL_PARAM_INPUT
#define DB2_PARAM_OUT SQL_PARAM_OUTPUT
#define DB2_PARAM_INOUT SQL_PARAM_INPUT_OUTPUT
#define DB2_PARAM_FILE 11
/******** end DB2 defines */

/*fetch*/
#define DB2_FETCH_INDEX	0x01
#define DB2_FETCH_ASSOC	0x02
#define DB2_FETCH_BOTH	0x03

/* Change column case */
#define DB2_CASE_NATURAL 0
#define DB2_CASE_LOWER 1
#define DB2_CASE_UPPER 2

/* Trusted context case */
#define DB2_TRUSTED_CONTEXT_ENABLE SQL_ATTR_USE_TRUSTED_CONTEXT
