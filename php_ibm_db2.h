/*

  +----------------------------------------------------------------------+
  | Copyright IBM Corporation 2005.                                      |
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
  | Author: Sushant Koduru                                               |
  +----------------------------------------------------------------------+

  $Id$
*/

#ifndef PHP_IBM_DB2_H
#define PHP_IBM_DB2_H

extern zend_module_entry ibm_db2_module_entry;
#define phpext_ibm_db2_ptr &ibm_db2_module_entry

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlcli1.h>

#ifdef PHP_WIN32
#define PHP_IBM_DB2_API __declspec(dllexport)
#else
#define PHP_IBM_DB2_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define DB2_MAX_ERR_MSG_LEN (SQL_MAX_MESSAGE_LENGTH + SQL_SQLSTATE_SIZE + 1)

/* Used in _php_parse_options */
#define DB2_ERRMSG 1
#define DB2_ERR 2

/******** Makes code compatible with the options used by the user */
#define DB2_BINARY 1
#define DB2_CONVERT 2
#define DB2_PASSTHRU 3

#define DB2_SCROLLABLE SQL_SCROLL_DYNAMIC
#define DB2_FORWARD_ONLY SQL_SCROLL_FORWARD_ONLY

#define DB2_AUTOCOMMIT_ON SQL_AUTOCOMMIT_ON
#define DB2_AUTOCOMMIT_OFF SQL_AUTOCOMMIT_OFF

#define DB2_PARAM_IN SQL_PARAM_INPUT
#define DB2_PARAM_OUT SQL_PARAM_OUTPUT
#define DB2_PARAM_INOUT SQL_PARAM_INPUT_OUTPUT
#define DB2_PARAM_FILE 11
/******** end DB2 defines */

/*fetch*/
#define DB2_FETCH_INDEX	0x01
#define DB2_FETCH_ASSOC	0x02
#define DB2_FETCH_BOTH	0x03


PHP_MINIT_FUNCTION(ibm_db2);
PHP_MSHUTDOWN_FUNCTION(ibm_db2);
PHP_MINFO_FUNCTION(ibm_db2);

PHP_FUNCTION(db2_connect);
PHP_FUNCTION(db2_commit);
PHP_FUNCTION(db2_pconnect);
PHP_FUNCTION(db2_autocommit);
PHP_FUNCTION(db2_bind_param);
PHP_FUNCTION(db2_close);
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
PHP_FUNCTION(db2_fetch_into);
PHP_FUNCTION(db2_fetch_both);
PHP_FUNCTION(db2_result_all);
PHP_FUNCTION(db2_free_result);
PHP_FUNCTION(db2_set_option);
PHP_FUNCTION(db2_setoption);
PHP_FUNCTION(db2_fetch_object);

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(ibm_db2)
	SQLHANDLE 	henv;
	int    		bin_mode;
	char		__php_conn_err_msg[DB2_MAX_ERR_MSG_LEN];
	char            __php_conn_err_state[SQL_SQLSTATE_SIZE + 1];
	char            __php_stmt_err_msg[DB2_MAX_ERR_MSG_LEN];
	char            __php_stmt_err_state[SQL_SQLSTATE_SIZE + 1];
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
