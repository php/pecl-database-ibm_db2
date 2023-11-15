/*

  +----------------------------------------------------------------------+
  | Copyright IBM Corporation 2005-2014                                  |
  +----------------------------------------------------------------------+
  |                                                                      |
  | Licensed under the Apache License, Version 2.0 (the "License"); you  |
  | may not use this file except in compliance with the License. You may |
  | obtain a copy of the License at                                      |
  | http://www.apache.org/licenses/LICENSE-2.0                           |
  |                                                                      |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied. See the License for the specific language governing         |
  | permissions and limitations under the License.                       |
  +----------------------------------------------------------------------+
  | Authors: Sushant Koduru, Lynh Nguyen, Kanchana Padmanabhan,          |
  |          Dan Scott, Helmut Tessarek, Ambrish Bhargava,               |
  |          Rahul Priyadarshi                                           |
  +----------------------------------------------------------------------+

  $Id$
*/

#define	PHP_IBM_DB2_VERSION	"2.2.0"

#ifndef PHP_IBM_DB2_H
#define PHP_IBM_DB2_H

extern zend_module_entry ibm_db2_module_entry;
#define phpext_ibm_db2_ptr &ibm_db2_module_entry

#ifdef PHP_WIN32
#define PHP_IBM_DB2_API __declspec(dllexport)
#else
#define PHP_IBM_DB2_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* This needs to be imported here so that we get SQL_SQLSTATE_SIZE */
#include <sqlcli1.h>

/* strlen(" SQLCODE=") added in */
#define DB2_MAX_ERR_MSG_LEN (SQL_MAX_MESSAGE_LENGTH + SQL_SQLSTATE_SIZE + 10)

PHP_MINIT_FUNCTION(ibm_db2);
PHP_MSHUTDOWN_FUNCTION(ibm_db2);
PHP_MINFO_FUNCTION(ibm_db2);
PHP_RSHUTDOWN_FUNCTION(ibm_db2);

PHP_FUNCTION(db2_connect);
PHP_FUNCTION(db2_commit);
PHP_FUNCTION(db2_pconnect);
PHP_FUNCTION(db2_autocommit);
PHP_FUNCTION(db2_bind_param);
PHP_FUNCTION(db2_close);
#ifdef PASE /* db2_pclose - last ditch persistent close */
PHP_FUNCTION(db2_pclose);
#endif /* PASE */
PHP_FUNCTION(db2_columnprivileges);
PHP_FUNCTION(db2_column_privileges);
PHP_FUNCTION(db2_columns);
PHP_FUNCTION(db2_foreignkeys);
PHP_FUNCTION(db2_foreign_keys);
PHP_FUNCTION(db2_primarykeys);
PHP_FUNCTION(db2_primary_keys);
PHP_FUNCTION(db2_procedure_columns);
PHP_FUNCTION(db2_procedures);
PHP_FUNCTION(db2_specialcolumns);
PHP_FUNCTION(db2_special_columns);
PHP_FUNCTION(db2_statistics);
PHP_FUNCTION(db2_tableprivileges);
PHP_FUNCTION(db2_table_privileges);
PHP_FUNCTION(db2_tables);
PHP_FUNCTION(db2_commit);
PHP_FUNCTION(db2_exec);
PHP_FUNCTION(db2_prepare);
PHP_FUNCTION(db2_execute);
#ifndef PASE /* i5/OS unsupported */
PHP_FUNCTION(db2_execute_many);
#endif /* PASE */
PHP_FUNCTION(db2_conn_errormsg);
PHP_FUNCTION(db2_stmt_errormsg);
PHP_FUNCTION(db2_conn_error);
PHP_FUNCTION(db2_stmt_error);
PHP_FUNCTION(db2_next_result);
PHP_FUNCTION(db2_num_fields);
PHP_FUNCTION(db2_num_rows);
PHP_FUNCTION(db2_field_name);
PHP_FUNCTION(db2_field_display_size);
PHP_FUNCTION(db2_field_num);
PHP_FUNCTION(db2_field_precision);
PHP_FUNCTION(db2_field_scale);
PHP_FUNCTION(db2_field_type);
PHP_FUNCTION(db2_field_width);
PHP_FUNCTION(db2_cursor_type);
PHP_FUNCTION(db2_rollback);
PHP_FUNCTION(db2_free_stmt);
PHP_FUNCTION(db2_result);
PHP_FUNCTION(db2_fetch_row);
PHP_FUNCTION(db2_fetch_assoc);
PHP_FUNCTION(db2_fetch_array);
PHP_FUNCTION(db2_fetch_both);
PHP_FUNCTION(db2_result_all);
PHP_FUNCTION(db2_free_result);
PHP_FUNCTION(db2_set_option);
PHP_FUNCTION(db2_setoption);
PHP_FUNCTION(db2_fetch_object);
PHP_FUNCTION(db2_server_info);
PHP_FUNCTION(db2_client_info);
PHP_FUNCTION(db2_escape_string);
PHP_FUNCTION(db2_lob_read);
PHP_FUNCTION(db2_get_option);
PHP_FUNCTION(db2_getoption);
PHP_FUNCTION(db2_last_insert_id);

/*
	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(ibm_db2)
	zend_long		bin_mode;
	char		__php_conn_err_msg[DB2_MAX_ERR_MSG_LEN];
	char		__php_conn_err_state[SQL_SQLSTATE_SIZE + 1];
	char		__php_stmt_err_msg[DB2_MAX_ERR_MSG_LEN];
	char		__php_stmt_err_state[SQL_SQLSTATE_SIZE + 1];
	zend_long		i5_allow_commit;	/* orig  - IBM i legacy CRTLIB containers fail under commit control (isolation *NONE) */
	zend_long		i5_sys_naming;		/* 1.9.7 - IBM i + LUW DB2 Connect 10.5 system naming (customer *LIBL issues) */
#ifdef PASE /* IBM i ibm_db2.ini options */
	zend_long		i5_dbcs_alloc;		/* orig  - IBM i 6x space for CCSID<>UTF-8 convert  (DBCS customer issue) */
	zend_long		i5_all_pconnect;	/* orig  - IBM i force all connect to pconnect (operator issue) */
	zend_long		i5_ignore_userid;	/* orig  - IBM i ignore user id enables no-QSQSRVR job (custom site request) */
	zend_long		i5_job_sort;		/* orig  - IBM i SQL_ATTR_JOB_SORT_SEQUENCE (customer request DB2 PTF) */
	zend_long		i5_override_ccsid;	/* 1.9.7 - IBM i force UTF-8 CCSID (DBCS customer issue) */
	zend_long		i5_blank_userid;	/* 1.9.7 - IBM i security restrict blank db,uid,pwd (unless customer allow flag) */
	zend_long		i5_log_verbose;		/* 1.9.7 - IBM i consultant request log additional information into php.log */
	zend_long		i5_max_pconnect;	/* 1.9.7 - IBM i count max usage connection recycle (customer issue months live connection) */
	zend_long		i5_check_pconnect;	/* 1.9.7 - IBM i remote persistent connection or long lived local (customer issue dead connection) */
	char		        *i5_servermode_subsystem; /* 1.9.7 - IBM i consultant request switch subsystem QSQSRVR job (customer workload issues) */
	zend_long		i5_guard_profile;	/* 1.9.7 - IBM i monitor switch user profile applications (customer security issue) */
	zend_long		i5_char_trim;		/* 2.0.3  - IBM i trim spaces character results  (customer size request issue) */
#endif /* PASE */
ZEND_END_MODULE_GLOBALS(ibm_db2)

/* In every utility function you add that needs to use variables
   in php_ibm_db2_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as IBM_DB2_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define IBM_DB2_G(v) TSRMG(ibm_db2_globals_id, zend_ibm_db2_globals *, v)
#else
#define IBM_DB2_G(v) (ibm_db2_globals.v)
#endif

#endif	/* PHP_IBM_DB2_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
