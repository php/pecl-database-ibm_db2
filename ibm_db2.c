/*
  +----------------------------------------------------------------------+
  | (C) Copyright IBM Corporation 2005.                                  |
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
  |          Dan Scott, Helmut Tessarek                                  |
  +----------------------------------------------------------------------+

  $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "../ext/standard/info.h"
#include "php_ibm_db2.h"

ZEND_DECLARE_MODULE_GLOBALS(ibm_db2)

#if ZEND_MODULE_API_NO > 20020429
#define ONUPDATEFUNCTION OnUpdateLong
#else
#define ONUPDATEFUNCTION OnUpdateInt
#endif

/* True global resources - no need for thread safety here */
static int le_conn_struct, le_stmt_struct, le_pconn_struct;

static void _php_db2_check_sql_errors( SQLHANDLE handle, SQLSMALLINT hType, int rc, int cpy_to_global, char* ret_str, int API, SQLSMALLINT recno TSRMLS_DC );
static void _php_db2_assign_options( void* handle, int type, char* opt_key, long data TSRMLS_DC );
static int _php_db2_parse_options( zval* options, int type, void* handle TSRMLS_DC );
static void _php_db2_clear_conn_err_cache(TSRMLS_D);
static void _php_db2_clear_stmt_err_cache(TSRMLS_D);
static char * _php_db2_instance_name;

/* Defines a linked list structure for caching param data */
typedef struct _param_cache_node {
	SQLSMALLINT	data_type;			/* Datatype */
	SQLUINTEGER	param_size;			/* param size */
	SQLSMALLINT nullable;			/* is Nullable */
	SQLSMALLINT	scale;				/* Decimal scale */
	SQLUINTEGER file_options;		/* File options if DB2_PARAM_FILE */
	int			param_num;			/* param number in stmt */
	int			param_type;			/* Type of param - INP/OUT/INP-OUT/FILE */
	char		*varname;			/* bound variable name */
	zval		*value;				/* Temp storage value */
	struct _param_cache_node *next;	/* Pointer to next node */
} param_node;

typedef struct _conn_handle_struct {
	SQLHANDLE hdbc;
	long auto_commit;
	long c_bin_mode;
	int handle_active;
	SQLSMALLINT error_recno_tracker;
	SQLSMALLINT errormsg_recno_tracker;
	int flag_pconnect; /* Indicates that this connection is persistent */
} conn_handle;

typedef union {
	int i_val;
	long l_val;
	double d_val;
	float f_val;
	short s_val;
	char *str_val;
} db2_row_data_type;

typedef struct {
	SQLINTEGER out_length;
	db2_row_data_type data;
} db2_row_type;

typedef struct _db2_result_set_info_struct {
	SQLCHAR		*name;
	SQLSMALLINT type;
	SQLUINTEGER size;
	SQLSMALLINT scale;
	SQLSMALLINT nullable;
} db2_result_set_info;

typedef struct _stmt_handle_struct {
	SQLHANDLE hdbc;
	SQLHANDLE hstmt;
	long s_bin_mode;
	long cursor_type;
	SQLSMALLINT error_recno_tracker;
	SQLSMALLINT errormsg_recno_tracker;

	/* Parameter Caching variables */
	param_node *head_cache_list;
	param_node *current_node;

	int num_params;				/* Number of Params */
	int file_param;				/* if option passed in is FILE_PARAM */
	int num_columns;
	db2_result_set_info *column_info;
	db2_row_type *row_data;
} stmt_handle;


/* equivalent functions on different platforms */
#ifdef PHP_WIN32
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

/* {{{ Every user visible function must have an entry in ibm_db2_functions[].
*/
function_entry ibm_db2_functions[] = {
	PHP_FE(db2_connect,	NULL)
	PHP_FE(db2_commit,	NULL)
	PHP_FE(db2_pconnect,	NULL)
	PHP_FE(db2_autocommit,	NULL)
	PHP_FE(db2_bind_param,	NULL)
	PHP_FE(db2_close,	NULL)
	PHP_FE(db2_column_privileges,	NULL)
	PHP_FALIAS(db2_columnprivileges, db2_column_privileges, NULL)
	PHP_FE(db2_columns,	NULL)
	PHP_FE(db2_foreign_keys,	NULL)
	PHP_FALIAS(db2_foreignkeys, db2_foreign_keys, NULL)
	PHP_FE(db2_primary_keys,	NULL)
	PHP_FALIAS(db2_primarykeys, db2_primary_keys,	NULL)
	PHP_FE(db2_procedure_columns,	NULL)
	PHP_FALIAS(db2_procedurecolumns, db2_procedure_columns,	NULL)
	PHP_FE(db2_procedures,	NULL)
	PHP_FE(db2_special_columns,	NULL)
	PHP_FALIAS(db2_specialcolumns, db2_special_columns,	NULL)
	PHP_FE(db2_statistics,	NULL)
	PHP_FE(db2_table_privileges,	NULL)
	PHP_FALIAS(db2_tableprivileges, db2_table_privileges,	NULL)
	PHP_FE(db2_tables,	NULL)
	PHP_FE(db2_exec,	NULL)
	PHP_FE(db2_prepare,	NULL)
	PHP_FE(db2_execute,	NULL)
	PHP_FE(db2_stmt_errormsg,	NULL)
	PHP_FE(db2_conn_errormsg,	NULL)
	PHP_FE(db2_conn_error,	NULL)
	PHP_FE(db2_stmt_error,	NULL)
	PHP_FE(db2_next_result,	NULL)
	PHP_FE(db2_num_fields,	NULL)
	PHP_FE(db2_num_rows,	NULL)
	PHP_FE(db2_field_name,	NULL)
	PHP_FE(db2_field_display_size,	NULL)
	PHP_FE(db2_field_num,	NULL)
	PHP_FE(db2_field_precision,	NULL)
	PHP_FE(db2_field_scale,	NULL)
	PHP_FE(db2_field_type,	NULL)
	PHP_FE(db2_field_width,	NULL)
	PHP_FE(db2_cursor_type,	NULL)
	PHP_FE(db2_rollback,	NULL)
	PHP_FE(db2_free_stmt,	NULL)
	PHP_FE(db2_result,	NULL)
	PHP_FE(db2_fetch_row,	NULL)
	PHP_FE(db2_fetch_assoc,	NULL)
	PHP_FE(db2_fetch_array,	NULL)
	PHP_FE(db2_fetch_both,	NULL)
	PHP_FE(db2_free_result,	NULL)
	PHP_FE(db2_set_option,  NULL)
	PHP_FALIAS(db2_setoption, db2_set_option,   NULL)
	PHP_FE(db2_fetch_object,    NULL)
	{NULL, NULL, NULL}	/* Must be the last line in ibm_db2_functions[] */
};
/* }}} */

/* {{{ ibm_db2_module_entry
*/
zend_module_entry ibm_db2_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"ibm_db2",
	ibm_db2_functions,
	PHP_MINIT(ibm_db2),
	PHP_MSHUTDOWN(ibm_db2),
	NULL,		/* Replace with NULL if there's nothing to do at request start */
	NULL,	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(ibm_db2),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IBM_DB2
ZEND_GET_MODULE(ibm_db2)
#endif

/* {{{ PHP_INI
*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("ibm_db2.binmode", "1", PHP_INI_ALL, ONUPDATEFUNCTION,
		bin_mode, zend_ibm_db2_globals, ibm_db2_globals)
	PHP_INI_ENTRY("ibm_db2.instance_name", NULL, PHP_INI_SYSTEM, NULL)
PHP_INI_END()
/* }}} */

/* {{{ static void php_ibm_db2_init_globals
*/
static void php_ibm_db2_init_globals(zend_ibm_db2_globals *ibm_db2_globals)
{
	/* env handle */
	ibm_db2_globals->henv = 0;
	ibm_db2_globals->bin_mode = 0;

	memset(ibm_db2_globals->__php_conn_err_msg, 0, DB2_MAX_ERR_MSG_LEN);
	memset(ibm_db2_globals->__php_stmt_err_msg, 0, DB2_MAX_ERR_MSG_LEN);
	memset(ibm_db2_globals->__php_conn_err_state, 0, SQL_SQLSTATE_SIZE + 1);
	memset(ibm_db2_globals->__php_stmt_err_state, 0, SQL_SQLSTATE_SIZE + 1);
}
/* }}} */

/* {{{ static void _php_db2_free_conn_struct */
static void _php_db2_free_conn_struct(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	int rc;

	conn_handle *handle = (conn_handle*) rsrc->ptr;
	/* Disconnect from DB. If stmt is allocated, it is freed automatically*/
	if ( handle->handle_active ) {
		rc = SQLDisconnect((SQLHDBC)handle->hdbc);
		rc = SQLFreeHandle(SQL_HANDLE_DBC, handle->hdbc);
	}
	if ( handle != NULL ) {
		if ( handle->flag_pconnect ) {
			/* Important to use regular free, we don't want the handled collected by efree */
			pefree(handle, 1);
		} else {
			efree(handle);
		}
	}
}
/* }}} */

/* {{{ static void _php_db2_free_pconn_struct */
static void _php_db2_free_pconn_struct(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	_php_db2_free_conn_struct(rsrc TSRMLS_CC);
}
/* }}} */

/* {{{ static void _php_db2_free_result_struct(stmt_handle* handle)
	*/
static void _php_db2_free_result_struct(stmt_handle* handle)
{
	int i;
	param_node *curr_ptr = NULL, *prev_ptr = NULL;

	if ( handle != NULL ) {
		/* Free param cache list */
		curr_ptr = handle->head_cache_list;
		prev_ptr = handle->head_cache_list;

		while (curr_ptr != NULL) {
			curr_ptr = curr_ptr->next;

			if (prev_ptr->varname) {
				efree(prev_ptr->varname);
			}

			/* Free Values */
			if ( Z_TYPE_P(prev_ptr->value) == IS_STRING ) {
				efree((prev_ptr->value)->value.str.val);
			}

			efree(prev_ptr);

			prev_ptr = curr_ptr;
		}
		/* free row data cache */
		if (handle->row_data) {
			for (i=0; i<handle->num_columns;i++) {
				switch (handle->column_info[i].type) {
				case SQL_CHAR:
				case SQL_VARCHAR:
				case SQL_LONGVARCHAR:
				case SQL_TYPE_DATE:
				case SQL_TYPE_TIME:
				case SQL_TYPE_TIMESTAMP:
				case SQL_BIGINT:
				case SQL_DECIMAL:
				case SQL_NUMERIC:
					if ( handle->row_data[i].data.str_val != NULL ) {
						efree(handle->row_data[i].data.str_val);
						handle->row_data[i].data.str_val = NULL;
					}

				}
			}
			efree(handle->row_data);
			handle->row_data = NULL;
		}

		/* free column info cache */
		if ( handle->column_info ) {
			for (i=0; i<handle->num_columns; i++) {
				efree(handle->column_info[i].name);
			}
			efree(handle->column_info);
			handle->column_info = NULL;
			handle->num_columns = 0;
		}
	}
}
/* }}} */

/* {{{ static stmt_handle *_db2_new_stmt_struct(conn_handle* conn_res)
	*/
static stmt_handle *_db2_new_stmt_struct(conn_handle* conn_res)
{
	stmt_handle *stmt_res;

	stmt_res = (stmt_handle *)emalloc(sizeof(stmt_handle));

	/* Initialize stmt resource so parsing assigns updated options if needed */
	stmt_res->hdbc = conn_res->hdbc;
	stmt_res->s_bin_mode = conn_res->c_bin_mode;
	stmt_res->cursor_type = DB2_FORWARD_ONLY;

	stmt_res->head_cache_list = NULL;
	stmt_res->current_node = NULL;

	stmt_res->num_params = 0;
	stmt_res->file_param = 0;

	stmt_res->column_info = NULL;
	stmt_res->num_columns = 0;

	stmt_res->error_recno_tracker = 1;
	stmt_res->errormsg_recno_tracker = 1;

	stmt_res->row_data = NULL;

	return stmt_res;
}
/* }}} */

/* {{{ static _php_db2_free_stmt_struct */
static void _php_db2_free_stmt_struct(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	int rc;

	stmt_handle *handle = (stmt_handle*) rsrc->ptr;

	rc = SQLFreeHandle( SQL_HANDLE_STMT, handle->hstmt);

	if ( handle ) {
		_php_db2_free_result_struct(handle);
		efree(handle);
	}
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(ibm_db2)
{
	/* Declare variables for DB2 instance settings */
	char * tmp_name;
	char * instance_name;

	ZEND_INIT_MODULE_GLOBALS(ibm_db2, php_ibm_db2_init_globals, NULL);

	REGISTER_LONG_CONSTANT("DB2_BINARY", 1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_CONVERT", 2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PASSTHRU", 3, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DB2_SCROLLABLE", SQL_SCROLL_DYNAMIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_FORWARD_ONLY", SQL_SCROLL_FORWARD_ONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PARAM_IN", SQL_PARAM_INPUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PARAM_OUT", SQL_PARAM_OUTPUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PARAM_INOUT", SQL_PARAM_INPUT_OUTPUT, CONST_CS | CONST_PERSISTENT);
	/* This number chosen is just a place holder to decide binding function to call */
	REGISTER_LONG_CONSTANT("DB2_PARAM_FILE", 11, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DB2_AUTOCOMMIT_ON", SQL_AUTOCOMMIT_ON, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_AUTOCOMMIT_OFF", SQL_AUTOCOMMIT_OFF, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DB2_DOUBLE", SQL_DOUBLE, CONST_CS | CONST_PERSISTENT);
	/* This is how CLI defines SQL_C_LONG */
	REGISTER_LONG_CONSTANT("DB2_LONG", SQL_INTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_CHAR", SQL_CHAR, CONST_CS | CONST_PERSISTENT);

	REGISTER_INI_ENTRIES();

#ifndef PHP_WIN32
	tmp_name = INI_STR("ibm_db2.instance_name");
	if (NULL != tmp_name) {
		instance_name = (char *)malloc(strlen(DB2_VAR_INSTANCE) + strlen(tmp_name) + 1);
		strcpy(instance_name, DB2_VAR_INSTANCE);
		strcat(instance_name, tmp_name);
		putenv(instance_name);
		_php_db2_instance_name = instance_name;
	}
#endif

	le_conn_struct = zend_register_list_destructors_ex( _php_db2_free_conn_struct, NULL, "conn struct", module_number);
	le_pconn_struct = zend_register_list_destructors_ex(NULL, _php_db2_free_pconn_struct, "pconn struct", module_number);
	le_stmt_struct = zend_register_list_destructors_ex( _php_db2_free_stmt_struct, NULL, "stmt struct", module_number);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(ibm_db2)
{
	UNREGISTER_INI_ENTRIES();

	if ( IBM_DB2_G(henv) ) {
		SQLFreeHandle ( SQL_HANDLE_ENV, IBM_DB2_G(henv) );
	}
	if (NULL != _php_db2_instance_name) {
		free(_php_db2_instance_name);
		_php_db2_instance_name = NULL;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(ibm_db2)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ibm_db2 support", "enabled");

	switch (IBM_DB2_G(bin_mode)) {
		case DB2_BINARY:
			php_info_print_table_row(2, "ibm_db2.binmode", "DB2_BINARY");
		break;	

		case DB2_CONVERT:
			php_info_print_table_row(2, "ibm_db2.binmode", "DB2_CONVERT");
		break;	

		case DB2_PASSTHRU:
			php_info_print_table_row(2, "ibm_db2.binmode", "DB2_PASSTHRU");
		break;	
	}
	php_info_print_table_row(2, "ibm_db2.instance_name", INI_STR("ibm_db2.instance_name"));
	php_info_print_table_end();

}
/* }}} */

/* {{{ static void _php_db2_init_error_info(stmt_handle *stmt_res)
*/
static void _php_db2_init_error_info(stmt_handle *stmt_res)
{
	stmt_res->error_recno_tracker = 1;
	stmt_res->errormsg_recno_tracker = 1;
}
/* }}} */

/* {{{ static void _php_db2_check_sql_errors( SQLHANDLE handle, SQLSMALLINT hType, int rc, int cpy_to_global, char* ret_str, int API SQLSMALLINT recno TSRMLS_DC)
*/
static void _php_db2_check_sql_errors( SQLHANDLE handle, SQLSMALLINT hType, int rc, int cpy_to_global, char* ret_str, int API, SQLSMALLINT recno TSRMLS_DC )
{
	SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH + 1];
	SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
	SQLCHAR errMsg[DB2_MAX_ERR_MSG_LEN];
	SQLINTEGER sqlcode;
	SQLSMALLINT length;
	char *p;

	if ( SQLGetDiagRec(hType, handle, recno, sqlstate, &sqlcode, msg,
			SQL_MAX_MESSAGE_LENGTH + 1, &length ) == SQL_SUCCESS) {

		p = strchr( msg, '\n' );
		if (p) *p = '\0';
		
		sprintf(errMsg, "%s SQLCODE=%d", msg, (int)sqlcode);
		errMsg[DB2_MAX_ERR_MSG_LEN] = '\0';

		switch (rc) {
			case SQL_ERROR:
				/* Need to copy the error msg and sqlstate into the symbol Table to cache these results */
				if ( cpy_to_global ) {
					switch (hType) {
						case SQL_HANDLE_DBC:
							strncpy(IBM_DB2_G(__php_conn_err_state), sqlstate, SQL_SQLSTATE_SIZE+1);
							strncpy(IBM_DB2_G(__php_conn_err_msg), errMsg, DB2_MAX_ERR_MSG_LEN);
							break;

						case SQL_HANDLE_STMT:
							strncpy(IBM_DB2_G(__php_stmt_err_state), sqlstate, SQL_SQLSTATE_SIZE+1);
							strncpy(IBM_DB2_G(__php_stmt_err_msg), errMsg, DB2_MAX_ERR_MSG_LEN);
							break;
					}
				}

				/* This call was made from db2_errmsg or db2_error */
				/* Check for error and return */
				switch (API) {
					case DB2_ERR:
						if ( ret_str != NULL ) {
							strncpy(ret_str, sqlstate, SQL_SQLSTATE_SIZE+1);
						}
						return;
					case DB2_ERRMSG:
						if ( ret_str != NULL ) {
							strncpy(ret_str, msg, DB2_MAX_ERR_MSG_LEN);
						}
						return;
					default:
						break;
				}

				break;

			default:
				break;
		}
	}
}
/* }}} */

/* {{{ static void _php_db2_assign_options( void *handle, int type, char* opt_key, long data )
	*/
static void _php_db2_assign_options( void *handle, int type, char *opt_key, long data TSRMLS_DC )
{
	int rc = 0;

	if ( !STRCASECMP(opt_key, "cursor")) {
		if ( type == SQL_HANDLE_STMT ) {
			if (((stmt_handle *)handle)->cursor_type != data ) {
				switch (data) {
					case DB2_SCROLLABLE:
						((stmt_handle *)handle)->cursor_type = DB2_SCROLLABLE;
						rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
							SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC,
							SQL_IS_INTEGER );
						break;

					case DB2_FORWARD_ONLY:
						rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
							SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_SCROLL_FORWARD_ONLY,
							SQL_IS_INTEGER );
						((stmt_handle *)handle)->cursor_type = DB2_FORWARD_ONLY;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect value passed in");
						break;
				}
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect Resource passed in");
		}
	} else if (!STRCASECMP(opt_key, "autocommit")) {
		if (type == SQL_HANDLE_DBC ) {
			if (((conn_handle *)handle)->auto_commit != data) {
				switch (data) {
					case DB2_AUTOCOMMIT_ON:
						/*	Setting AUTOCOMMIT again here. The user could modify
							this option, close the connection, and reopen it again
							with this option.
						*/
						((conn_handle*)handle)->auto_commit = 1;
						rc = SQLSetConnectAttr((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_NTS);
						break;

					case DB2_AUTOCOMMIT_OFF:
						((conn_handle*)handle)->auto_commit = 0;
						rc = SQLSetConnectAttr((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect value passed in");
						break;
				}
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect Resource passed in");
		}
	} else if (!STRCASECMP(opt_key, "binmode")) {
		switch (data) {
			/* TODO: Assign the Binary options here using CLI calls */
			case DB2_BINARY:
				switch (type) {
					case SQL_HANDLE_DBC:
						((conn_handle*)handle)->c_bin_mode = DB2_BINARY;
						break;

					case SQL_HANDLE_STMT:
						((stmt_handle *)handle)->s_bin_mode = DB2_BINARY;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect resource passed in\n");
				}
				break;
			case DB2_PASSTHRU:
				switch (type) {
					case SQL_HANDLE_DBC:
						((conn_handle*)handle)->c_bin_mode = DB2_PASSTHRU;
						break;

					case SQL_HANDLE_STMT:
						((stmt_handle *)handle)->s_bin_mode = DB2_PASSTHRU;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect resource passed in\n");
				}
				break;
			case DB2_CONVERT:
				switch (type) {
					case SQL_HANDLE_DBC:
						((conn_handle*)handle)->c_bin_mode = DB2_CONVERT;
						break;

					case SQL_HANDLE_STMT:
						((stmt_handle *)handle)->s_bin_mode = DB2_CONVERT;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect resource passed in\n");
				}
				break;
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect option setting passed in");
	}
}
/* }}} */

/* {{{ static int _php_db2_parse_options( zval *options, int type, void *handle)
*/
static int _php_db2_parse_options ( zval *options, int type, void *handle TSRMLS_DC )
{
	int numOpts = 0, i = 0;
	ulong num_idx;
	char *opt_key; /* Holds the Option Index Key */
	zval **data;

	if ( options != NULL) {
		numOpts = zend_hash_num_elements(Z_ARRVAL_P(options));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));

		for ( i = 0; i < numOpts; i++) {
			if (zend_hash_get_current_key(Z_ARRVAL_P(options), &opt_key,
				&num_idx, 1) == HASH_KEY_IS_STRING) {

				zend_hash_get_current_data(Z_ARRVAL_P(options), (void**)&data);

				/* Assign options to handle. */
				/* Sets the options in the handle with CLI/ODBC calls */
				_php_db2_assign_options( handle, type, opt_key, Z_LVAL_PP(data) TSRMLS_CC );

				zend_hash_move_forward(Z_ARRVAL_P(options));

				/* Since zend_hash_get_current_key assigns memory to this
					string each time it is called */
				if (opt_key) {
					efree(opt_key);
				}
			} else {
				return -1;
			}
		}
	}
	return 0;
}
/* }}} */

/* {{{ static int _php_db2_get_result_set_info(stmt_handle *stmt_res)
initialize the result set information of each column. This must be done once
*/
static int _php_db2_get_result_set_info(stmt_handle *stmt_res)
{
	int rc = -1, i;
	SQLSMALLINT nResultCols = 0, name_length;
	char tmp_name[BUFSIZ];
	rc = SQLNumResultCols((SQLHSTMT)stmt_res->hstmt, &nResultCols);
	if ( rc == SQL_ERROR || nResultCols == 0) {
		return -1;
	}
	stmt_res->num_columns = nResultCols;
	stmt_res->column_info = (db2_result_set_info*)emalloc(sizeof(db2_result_set_info)*nResultCols);
	/* return a set of attributes for a column */
	for (i = 0 ; i < nResultCols; i++) {
		rc = SQLDescribeCol((SQLHSTMT)stmt_res->hstmt, (SQLSMALLINT)(i + 1 ),
			tmp_name, BUFSIZ, &name_length, &stmt_res->column_info[i].type,
			&stmt_res->column_info[i].size, &stmt_res->column_info[i].scale,
			&stmt_res->column_info[i].nullable);
		if ( rc == SQL_ERROR ) {
			return -1;
		}
		if ( name_length <= 0 ) {
			stmt_res->column_info[i].name = estrdup("");
		} else if (name_length >= BUFSIZ ) {
			/* column name is longer than BUFSIZ*/
			stmt_res->column_info[i].name = emalloc(name_length+1);
			rc = SQLDescribeCol((SQLHSTMT)stmt_res->hstmt, (SQLSMALLINT)(i + 1 ),
				stmt_res->column_info[i].name, name_length, &name_length,
				&stmt_res->column_info[i].type, &stmt_res->column_info[i].size,
				&stmt_res->column_info[i].scale, &stmt_res->column_info[i].nullable);
			if ( rc == SQL_ERROR ) {
				return -1;
			}
		} else {
			stmt_res->column_info[i].name = estrdup(tmp_name);
		}

	}
	return 0;
}
/* }}} */

/* {{{ static int _php_db2_bind_column_helper(stmt_handle *stmt_res)
	bind columns to data, this must be done once
*/
static int _php_db2_bind_column_helper(stmt_handle *stmt_res)
{
	SQLINTEGER in_length;
	SQLSMALLINT column_type;
	db2_row_data_type *row_data;
	int i, rc = SQL_SUCCESS;

	stmt_res->row_data = (db2_row_type*) emalloc(sizeof(db2_row_type)
		* stmt_res->num_columns);
	memset(stmt_res->row_data,0x0,sizeof(db2_row_type)*stmt_res->num_columns);

	for (i=0; i<stmt_res->num_columns; i++) {
		column_type = stmt_res->column_info[i].type;
		row_data = &stmt_res->row_data[i].data;
		switch(column_type) {
			case SQL_CHAR:
			case SQL_VARCHAR:
			case SQL_LONGVARCHAR:
				in_length = stmt_res->column_info[i].size+1;
				row_data->str_val = (char*)emalloc(in_length);

				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_DEFAULT, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_BINARY:
			case SQL_LONGVARBINARY:
			case SQL_VARBINARY:
				if ( stmt_res->s_bin_mode == DB2_CONVERT ) {
					in_length = 2*(stmt_res->column_info[i].size)+1;
					row_data->str_val = (char*)emalloc(in_length);
	
					rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
						SQL_C_CHAR, row_data->str_val, in_length,
						(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				} else {
					in_length = stmt_res->column_info[i].size+1;
					row_data->str_val = (char*)emalloc(in_length);
	
					rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
						SQL_C_DEFAULT, row_data->str_val, in_length,
						(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				}
				break;

			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
			case SQL_TYPE_TIMESTAMP:
			case SQL_BIGINT:
				in_length = stmt_res->column_info[i].size+1;
				row_data->str_val = (char*)emalloc(in_length);
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_CHAR, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_SMALLINT:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_DEFAULT, &row_data->s_val, sizeof(row_data->s_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_INTEGER:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_DEFAULT, &row_data->i_val, sizeof(row_data->i_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_REAL:
			case SQL_FLOAT:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_DEFAULT, &row_data->f_val, sizeof(row_data->f_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_DOUBLE:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_DEFAULT, &row_data->d_val, sizeof(row_data->d_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_DECIMAL:
			case SQL_NUMERIC:
				in_length = stmt_res->column_info[i].size +
					stmt_res->column_info[i].scale + 2 + 1;
				row_data->str_val = (char *)emalloc(in_length);
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i+1),
					SQL_C_CHAR, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				break;

			case SQL_CLOB:
			case SQL_BLOB:
				stmt_res->row_data[i].out_length = 0;
				/* we do getdata call instead */
				break;

			default:
				break;
			}
		}
	return rc;
}
/* }}} */

/* {{{ static void _php_db2_clear_stmt_err_cache (TSRMLS_D)
*/
static void _php_db2_clear_stmt_err_cache(TSRMLS_D)
{
	memset(IBM_DB2_G(__php_stmt_err_msg), 0, DB2_MAX_ERR_MSG_LEN);
	memset(IBM_DB2_G(__php_stmt_err_state), 0, SQL_SQLSTATE_SIZE + 1);
}
/* }}} */

/* {{{ static int _php_db2_connect_helper( INTERNAL_FUNCTION_PARAMETERS, resource )
*/
static int _php_db2_connect_helper( INTERNAL_FUNCTION_PARAMETERS, conn_handle **pconn_res, int isPersistent )
{
	char *database = NULL;
	char *uid = NULL;
	char *password = NULL;
	int argc = ZEND_NUM_ARGS();
	long database_len;
	long uid_len;
	long password_len;
	zval *options = NULL;
	int rc = 0;
	SQLINTEGER conn_alive = 1;
	conn_handle *conn_res = *pconn_res;
	int reused = 0;
	int hKeyLen = 0;
	char *hKey = NULL;


	SQLHANDLE pHenv = 0;

	if (zend_parse_parameters(argc TSRMLS_CC, "sss|a", &database, &database_len,&uid,
		&uid_len, &password, &password_len, &options) == FAILURE) {
		return -1;
	}
	do {
		/* Check if we already have a connection for this userID & database combination */
		if (isPersistent) {
			list_entry *entry;
			hKeyLen = strlen(database) + strlen(uid) + strlen(password) + 9;
			hKey = (char *) emalloc(hKeyLen);

			sprintf(hKey, "__db2_%s.%s.%s", uid, database, password);

			if (zend_hash_find(&EG(persistent_list), hKey, hKeyLen, (void **) &entry) == SUCCESS) {
				conn_res = *pconn_res = (conn_handle *) entry->ptr;

				/* Need to reinitialize connection? */
				rc = SQLGetConnectAttr(conn_res->hdbc, SQL_ATTR_PING_DB, (SQLPOINTER)&conn_alive, 0, NULL); 
				if ( (rc == SQL_SUCCESS) && conn_alive ) {
					reused = 1;
				} /* else will re-connect since connection is dead */
			}
		} else {
			/* Need to check for max pconnections? */
		}
		if (*pconn_res == NULL) {
			conn_res = *pconn_res =
				(conn_handle *) (isPersistent ?  pemalloc(sizeof(conn_handle), 1) : emalloc(sizeof(conn_handle)));
			memset((void *) conn_res, '\0', sizeof(conn_handle));
		}
		/* We need to set this early, in case we get an error below,
			so we know how to free the connection */
		conn_res->flag_pconnect = isPersistent;
		/* Allocate ENV handles if not present */
		if ( !IBM_DB2_G(henv) ) {
			rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &pHenv);
			if (rc == SQL_ERROR) {
				_php_db2_check_sql_errors( pHenv, SQL_HANDLE_ENV, rc, 1, NULL, -1, 1 TSRMLS_CC);
				break;
			}
			/* enable connection pooling */
			rc = SQLSetEnvAttr((SQLHENV)pHenv, SQL_ATTR_CONNECTION_POOLING, (void*)SQL_CP_ONE_PER_HENV, 0);
			rc = SQLSetEnvAttr((SQLHENV)pHenv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

			IBM_DB2_G(henv) = pHenv;
		} else {
			pHenv = IBM_DB2_G(henv);
		}

		if (! reused) {
			/* Alloc CONNECT Handle */
			rc = SQLAllocHandle( SQL_HANDLE_DBC, pHenv, &(conn_res->hdbc));
			if (rc == SQL_ERROR) {
				_php_db2_check_sql_errors(pHenv, SQL_HANDLE_ENV, rc, 1, NULL, -1, 1 TSRMLS_CC);
				break;
			}
		}

		/* Set this after the connection handle has been allocated to avoid
		unnecessary network flows. Initialize the structure to default values */
		conn_res->auto_commit = DB2_AUTOCOMMIT_ON;
		rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_NTS);

		/* TODO: Set this option with the DB2 CLI at this point as well */
		conn_res->c_bin_mode = IBM_DB2_G(bin_mode);

		conn_res->error_recno_tracker = 1;
		conn_res->errormsg_recno_tracker = 1;

		/* handle not active as of yet */
		conn_res->handle_active = 0;

		/* Set Options */
		if ( options != NULL ) {
			rc = _php_db2_parse_options( options, SQL_HANDLE_DBC, conn_res TSRMLS_CC );
			if (rc == SQL_ERROR) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Options Array must have string indexes");
			}
		}

		if (! reused) {
			/* Connect */
			/* If the string contains a =, use SQLDriverConnect */
			if ( strstr(database, "=") != NULL ) {
				rc = SQLDriverConnect((SQLHDBC)conn_res->hdbc, (SQLHWND)NULL,
						(SQLCHAR*)database, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );
			} else {
				rc = SQLConnect( (SQLHDBC)conn_res->hdbc, (SQLCHAR *)database,
						(SQLSMALLINT)database_len, (SQLCHAR *)uid, (SQLSMALLINT)uid_len,
						(SQLCHAR *)password, (SQLSMALLINT)password_len );
			}

			if ( rc == SQL_ERROR) {
				_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				break;
			}
		}
		conn_res->handle_active = 1;
	} while (0);

	if (hKey != NULL) {
		if (! reused && rc != SQL_ERROR) {
			/* If we created a new persistent connection, add it to the persistent_list */
			list_entry newEntry;
			memset(&newEntry, '\0', sizeof(newEntry));
			Z_TYPE(newEntry) = le_pconn_struct;
			newEntry.ptr = conn_res;
			if (zend_hash_update(&EG(persistent_list), hKey, hKeyLen, (void *) &newEntry, sizeof(list_entry), NULL)==FAILURE) {
				rc = SQL_ERROR;
				/* TBD: What error to return?, for now just generic SQL_ERROR */
			}
		}
		efree(hKey);
	}
	return rc;
}
/* }}} */

/* {{{ static void _php_db2_clear_conn_err_cache (TSRMLS_D)
*/
static void _php_db2_clear_conn_err_cache(TSRMLS_D)
{
	/* Clear out the cached conn messages */
	memset(IBM_DB2_G(__php_conn_err_msg), 0, DB2_MAX_ERR_MSG_LEN);
	memset(IBM_DB2_G(__php_conn_err_state), 0, SQL_SQLSTATE_SIZE + 1);
}
/* }}} */

/* {{{ proto resource db2_connect(string database, string uid, string password [, array options])
Returns a connection to a database */
PHP_FUNCTION(db2_connect)
{
	int rc;

	conn_handle *conn_res = NULL;

	_php_db2_clear_conn_err_cache(TSRMLS_C);


	rc = _php_db2_connect_helper( INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_res, 0 );

	if ( rc == SQL_ERROR ) {
		if (conn_res != NULL && conn_res->handle_active) {
			rc = SQLFreeHandle( SQL_HANDLE_DBC, conn_res->hdbc);
		}

		/* free memory */
		if (conn_res != NULL) {
			efree(conn_res);
		}

		RETVAL_FALSE;
		return;
	} else {
		ZEND_REGISTER_RESOURCE(return_value, conn_res, le_conn_struct);
	}
}
/* }}} */

/* {{{ proto resource db2_pconnect(string database_name, string username, string password [, array options])
Returns a persistent connection to a database */
PHP_FUNCTION(db2_pconnect)
{
	int rc;
	conn_handle *conn_res = NULL;

	_php_db2_clear_conn_err_cache(TSRMLS_C);

	rc = _php_db2_connect_helper( INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_res, 1);

	if ( rc == SQL_ERROR ) {
		if (conn_res != NULL && conn_res->handle_active) {
			rc = SQLFreeHandle( SQL_HANDLE_DBC, conn_res->hdbc);
		}

		/* free memory */
		if (conn_res != NULL) {
			pefree(conn_res, 1);
		}

		RETVAL_FALSE;
		return;
	} else {
		ZEND_REGISTER_RESOURCE(return_value, conn_res, le_pconn_struct);
	}
}
/* }}} */

/* {{{ proto mixed db2_autocommit(resource connection[, bool value])
Returns or sets the AUTOCOMMIT state for a database connection */
PHP_FUNCTION(db2_autocommit)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval value;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc;
	SQLINTEGER autocommit;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|b", &connection, &value) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		/* If value in handle is different from value passed in */
		if (argc == 2) {
			autocommit = Z_BVAL(value);
			if(autocommit != (conn_res->auto_commit)) {
				rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)autocommit, SQL_IS_INTEGER);
				conn_res->auto_commit = autocommit;
			}
			RETURN_TRUE;
		} else {
			RETURN_LONG(conn_res->auto_commit);
		}
	}
}
/* }}} */

/* {{{ static void _php_db2_add_param_cache( stmt_handle *stmt_res, int param_no, char *varname, int param_type, SQLSMALLINT data_type, SQLSMALLINT precision, SQLSMALLINT scale, SQLSMALLINT nullable )
*/
static void _php_db2_add_param_cache( stmt_handle *stmt_res, int param_no, char *varname, int varname_len, int param_type, SQLSMALLINT data_type, SQLUINTEGER precision, SQLSMALLINT scale, SQLSMALLINT nullable )
{
	param_node *tmp_curr = NULL, *prev = NULL, *curr = stmt_res->head_cache_list;

	while ( (curr != NULL) && (curr->param_num < param_no) ) {
		prev = curr;
		curr = curr->next;
	}

	/* TODO: Combine the following two if's into a single block */
	if ( prev == NULL ) {
		/* Allocate memory and make new node to be added */
		tmp_curr = (param_node *)emalloc(sizeof(param_node));
		/* assign values */
		tmp_curr->data_type = data_type;
		tmp_curr->param_size = precision;
		tmp_curr->nullable = nullable;
		tmp_curr->scale = scale;
		tmp_curr->param_num = param_no;
		tmp_curr->file_options = SQL_FILE_READ;
		tmp_curr->param_type = param_type;
		/* Set this flag is stmt_res if a FILE INPUT is present */
		if ( param_type == DB2_PARAM_FILE) {
			stmt_res->file_param = 1;
		}

		MAKE_STD_ZVAL(tmp_curr->value);

		if( varname != NULL) {
			tmp_curr->varname = estrndup(varname, varname_len);
		}

		tmp_curr->next = curr;
		stmt_res->head_cache_list = tmp_curr;

		/* Increment num params added */
		stmt_res->num_params++;
	} else {
		if (prev->param_num != param_no) {
			/* Allocate memory and make new node to be added */
			tmp_curr = (param_node *)emalloc(sizeof(param_node));
			/* assign values */
			tmp_curr->data_type = data_type;
			tmp_curr->param_size = precision;
			tmp_curr->nullable = nullable;
			tmp_curr->scale = scale;
			tmp_curr->param_num = param_no;
			tmp_curr->file_options = SQL_FILE_READ;
			tmp_curr->param_type = param_type;

			/* Set this flag in stmt_res if a FILE INPUT is present */
			if ( param_type == DB2_PARAM_FILE) {
				stmt_res->file_param = 1;
			}

			if ( varname != NULL) {
				tmp_curr->varname = estrndup(varname, varname_len);
			}

			MAKE_STD_ZVAL(tmp_curr->value);

			tmp_curr->next = curr;
			prev->next = tmp_curr;

			/* Increment num params added */
			stmt_res->num_params++;
		} else {
			/* Both the nodes are for the same param no */
			/* Replace Information */
			curr->data_type = data_type;
			curr->param_size = precision;
			curr->nullable = nullable;
			curr->scale = scale;
			curr->param_num = param_no;
			curr->file_options = SQL_FILE_READ;
			curr->param_type = param_type;

			/* Set this flag in stmt_res if a FILE INPUT is present */
			if ( param_type == DB2_PARAM_FILE) {
				stmt_res->file_param = 1;
			}

			/* Free and assign the variable name again */
			/* Var lengths can be variable and different in both cases. */
			/* This shouldn't happen often anyway */
			if ( varname != NULL) {
				efree(curr->varname);
				curr->varname = estrndup(varname, varname_len);
			}
		}
	}
}
/* }}} */

/* {{{ proto bool db2_bind_param(resource stmt, long param_no, string varname [, long param_type [, long data_type [, long precision [, long scale]]]])
Binds a PHP variable to an SQL statement parameter */
PHP_FUNCTION(db2_bind_param)
{
	char *varname = NULL;
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	int varname_len;
	long param_type; /* set default here */
	/* LONG types used for data being passed in */
	long param_no;
	long data_type;
	long precision;
	long scale;
	SQLSMALLINT sql_data_type = 0;
	SQLUINTEGER sql_precision = 0;
	SQLSMALLINT sql_scale = 0;
	SQLSMALLINT sql_nullable = SQL_NO_NULLS;

	zval *stmt = NULL;
	stmt_handle *stmt_res;
	int rc = 0;

	if (zend_parse_parameters(argc TSRMLS_CC, "rls|llll", &stmt, &param_no,
		&varname, &varname_len, &param_type, &data_type, &precision,
		&scale) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE2(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct, le_pconn_struct);

		/* Check for Param options */
		switch (argc) {
			/* if argc == 3, then the default value for param_type will be used */
			case 3:
				param_type = DB2_PARAM_IN;
				/* Fall through */

			/* Otherwise, param_type will contain the value passed in */
			case 4:
			case 5:
			case 6:
				/* No param data specified */
				rc = SQLDescribeParam((SQLHSTMT)stmt_res->hstmt, param_no, &sql_data_type, &sql_precision, &sql_scale, &sql_nullable);
				if ( rc == SQL_ERROR ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Describe Param Failed");
					RETURN_FALSE;
				}
				/* Add to cache */
				_php_db2_add_param_cache( stmt_res, param_no, varname, varname_len, param_type, sql_data_type, sql_precision, sql_scale, sql_nullable );
				break;

			case 7:
				/* Cache param data passed */
				/* I am using a linked list of nodes here because I dont know before hand how many params are being passed in/bound. */
				/* To determine this, a call to SQLNumParams is necessary. This is take away any advantages an array would have over linked list access */
				/* Data is being copied over to the correct types for subsequent CLI call because this might cause problems on other platforms such as AIX */
				sql_data_type = (SQLSMALLINT)data_type;
				sql_precision = (SQLUINTEGER)precision;
				sql_scale = (SQLSMALLINT)scale;
				_php_db2_add_param_cache( stmt_res, param_no, varname, varname_len, param_type, sql_data_type, sql_precision, sql_scale, sql_nullable );
				break;

			default:
				WRONG_PARAM_COUNT;
				RETURN_FALSE;
		}
		/* end Switch */

		/* We bind data with DB2 CLI in db2_execute() */
		/* This will save network flow if we need to override params in it */

		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool db2_close(resource connection)
Closes a database connection */
PHP_FUNCTION(db2_close)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		/* Check to see if it's a persistent connection; if so, just return true */
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		if ( conn_res->handle_active && !conn_res->flag_pconnect ) {
			/* Disconnect from DB. If stmt is allocated, it is freed automatically */
			if (conn_res->auto_commit == 0) {
				rc = SQLEndTran(SQL_HANDLE_DBC, (SQLHDBC)conn_res->hdbc, SQL_ROLLBACK);
				if ( rc == SQL_ERROR )
					RETURN_FALSE;
			}
			rc = SQLDisconnect((SQLHDBC)conn_res->hdbc);
			if ( rc == SQL_ERROR ) {
				RETURN_FALSE;
			}

			rc = SQLFreeHandle( SQL_HANDLE_DBC, conn_res->hdbc);
			if ( rc == SQL_ERROR ) {
				RETURN_FALSE;
			}

			conn_res->handle_active = 0;

			RETURN_TRUE;
		} else if ( conn_res->flag_pconnect ) {
			/* Do we need to call FreeStmt or something to close cursors? */
			RETURN_TRUE;
		} else {
			RETURN_FALSE;
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_column_privileges(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the columns and associated privileges for a table */
PHP_FUNCTION(db2_column_privileges)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	char *column_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	int column_name_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|ssss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len,
		&column_name, &column_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);
		if (!conn_res) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Conn Resource cannot be found");
			RETURN_FALSE;
		}

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLColumnPrivileges((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS, column_name,SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_columns(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the columns and associated metadata for a table */
PHP_FUNCTION(db2_columns)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	char *column_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	int column_name_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|ssss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len,
		&column_name, &column_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLColumns((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS, column_name,SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_foreign_keys(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the foreign keys for a table */

PHP_FUNCTION(db2_foreign_keys)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;


	if (zend_parse_parameters(argc TSRMLS_CC, "rsss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLForeignKeys((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS, "",SQL_NTS,
						"",SQL_NTS,"",SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);

	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_primary_keys(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing primary keys for a table */

PHP_FUNCTION(db2_primary_keys)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "rsss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLPrimaryKeys((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);

	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_procedure_columns(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the input and output parameters for a stored procedure */
PHP_FUNCTION(db2_procedure_columns)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *proc_name = NULL;
	char *column_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int qualifier_len;
	int owner_len;
	int proc_name_len;
	int column_name_len;
	int connection_id = -1;
	int rc = 0;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;

	if (zend_parse_parameters(argc TSRMLS_CC, "rssss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &proc_name, &proc_name_len,
		&column_name, &column_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE(conn_res, conn_handle*, &connection, connection_id, "Connection Resource", le_conn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLProcedureColumns((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, proc_name, SQL_NTS, column_name, SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_procedures(resource connection, string qualifier, string owner, string proc_name)
Returns a result set listing the stored procedures registered in a database */
PHP_FUNCTION(db2_procedures)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *proc_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int qualifier_len;
	int owner_len;
	int proc_name_len;
	zval *connection = NULL;
	int connection_id = -1;
	int rc = 0;
	conn_handle *conn_res;
	stmt_handle *stmt_res;

	if (zend_parse_parameters(argc TSRMLS_CC, "rsss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &proc_name, &proc_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE(conn_res, conn_handle*, &connection, connection_id, "Connection Resource", le_conn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLProcedures((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, proc_name, SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_special_columns(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the unique row identifier columns for a table */
PHP_FUNCTION(db2_special_columns)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	int scope;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "rsssl", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len,&scope) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLSpecialColumns((SQLHSTMT)stmt_res->hstmt,SQL_BEST_ROWID, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS,scope,SQL_NULLABLE);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);

	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_statistics(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the index and statistics for a table */
PHP_FUNCTION(db2_statistics)
{
char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int qualifier_len;
	int owner_len;
	int table_name_len;
	zval unique;
	zval *connection = NULL;
	int connection_id = -1;
	int rc = 0;
	SQLUSMALLINT sql_unique;
	conn_handle *conn_res;
	stmt_handle *stmt_res;

	if (zend_parse_parameters(argc TSRMLS_CC, "rsssb", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len,
		&unique) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE(conn_res, conn_handle*, &connection, connection_id, "Connection Resource", le_conn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);
		sql_unique = Z_BVAL(unique);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLStatistics((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, table_name, SQL_NTS, sql_unique, SQL_QUICK);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_table_privileges(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the tables and associated privileges in a database */

PHP_FUNCTION(db2_table_privileges)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|sss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);
		if (!conn_res) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Conn Resource cannot be found");
			RETURN_FALSE;
		}

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLTablePrivileges((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, table_name, SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource db2_tables(resource connection, string qualifier, string owner, string table_name, string table_type)
Returns a result set listing the tables and associated metadata in a database */
PHP_FUNCTION(db2_tables)
{
	char *qualifier = NULL;
	char *owner = NULL;
	char *table_name = NULL;
	char *table_type = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	int table_type_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|ssss", &connection, &qualifier,
		&qualifier_len, &owner, &owner_len, &table_name, &table_name_len,
		&table_type, &table_type_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			RETURN_FALSE;
		}
		rc = SQLTables((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, table_name, SQL_NTS, table_type, SQL_NTS);
		if (rc == SQL_ERROR ) {
			RETURN_FALSE;
		}
		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool db2_commit(resource connection)
Commits a transaction */
PHP_FUNCTION(db2_commit)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		rc = SQLEndTran(SQL_HANDLE_DBC, conn_res->hdbc, SQL_COMMIT);

		if ( rc == SQL_ERROR ) {
			RETVAL_FALSE;
			return;
		} else {
			RETURN_TRUE;
		}
	}
}
/* }}} */

/* {{{ static int _php_db2_do_prepare(SQLHANDLE hdbc, string stmt_string, stmt_handle *stmt_res, zval *options TSRMLS_DC)
*/
static int _php_db2_do_prepare(SQLHANDLE hdbc, char* stmt_string, stmt_handle *stmt_res, zval *options TSRMLS_DC)
{
	int rc;

	/* alloc handle and return only if it errors */
	rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &(stmt_res->hstmt));
	if ( rc < SQL_SUCCESS ) {
		_php_db2_check_sql_errors(hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		return rc;
	}

	if (options != NULL) {
		rc = _php_db2_parse_options( options, SQL_HANDLE_STMT, stmt_res TSRMLS_CC );
		if ( rc == SQL_ERROR ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Options Array must have string indexes");
		}
	}

	/* Prepare the stmt. The cursor type requested has already been set in _php_db2_assign_options */
	rc = SQLPrepare((SQLHSTMT)stmt_res->hstmt, (SQLCHAR*)stmt_string, SQL_NTS);
	if ( rc == SQL_ERROR ) {
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
	}

	return rc;
}
/* }}} */

/* {{{ static int _php_db2_execute_stmt(conn_handle *conn_res, stmt_handle *stmt_res TSRMLS_DC)
*/
static int _php_db2_execute_stmt(stmt_handle *stmt_res TSRMLS_DC)
{
	int rc;

	rc = SQLExecute((SQLHSTMT)stmt_res->hstmt);
	if ( rc == SQL_ERROR ) {
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
	}

	return rc;
}
/* }}} */

/* {{{ proto resource db2_exec(resource connection, string stmt_string [, array options])
Executes an SQL statement directly */
PHP_FUNCTION(db2_exec)
{
	char *stmt_string = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int stmt_string_len;
	zval *connection = NULL;
	zval *options = NULL;
	stmt_handle *stmt_res;
	conn_handle *conn_res;
	int rc;

	/* This function basically is a wrap of the _php_db2_do_prepare and _php_db2_execute_stmt */
	/* After completing statement execution, it returns the statement resource */
	/* This function does not support parameter binding */
	if (zend_parse_parameters(argc TSRMLS_CC, "rs|a", &connection, &stmt_string,
		&stmt_string_len, &options) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		_php_db2_clear_stmt_err_cache(TSRMLS_C);

		stmt_res = _db2_new_stmt_struct(conn_res);

		/* Allocates the stmt handle */
		/* Prepares the statement */
		/* returns the stat_handle back to the calling function */
		rc = _php_db2_do_prepare(conn_res->hdbc, stmt_string, stmt_res, options TSRMLS_CC);
		if ( rc < SQL_SUCCESS ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Prepare Failed");
			efree(stmt_res);
			RETURN_FALSE;
		}

		rc = _php_db2_execute_stmt(stmt_res TSRMLS_CC);
		if ( rc < SQL_SUCCESS ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
			efree(stmt_res);
			RETURN_FALSE;
		}

		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	}
}
/* }}} */

/* {{{ proto bool db2_free_result(resource)
Frees resources associated with a result set */
PHP_FUNCTION(db2_free_result)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res;
	int rc = 0;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
		if ( stmt_res->hstmt ) {
			rc = SQLFreeHandle( SQL_HANDLE_STMT, stmt_res->hstmt);
			stmt_res->hstmt = 0;
		}
		_php_db2_free_result_struct(stmt_res);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource db2_prepare(resource connection, string stmt_string [, array options])
Prepares an SQL statement */
PHP_FUNCTION(db2_prepare)
{
	char *stmt_string = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int stmt_string_len;
	zval *connection = NULL;
	zval *options = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "rs|a", &connection, &stmt_string,
		&stmt_string_len, &options) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		_php_db2_clear_stmt_err_cache(TSRMLS_C);

		/* Initialize stmt resource members with default values. */
		/* Parsing will update options if needed */

		stmt_res = _db2_new_stmt_struct(conn_res);

		/* Allocates the stmt handle */
		/* Prepares the statement */
		/* returns the stat_handle back to the calling function */
		rc = _php_db2_do_prepare(conn_res->hdbc, stmt_string, stmt_res, options TSRMLS_CC);
		if ( rc < SQL_SUCCESS ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Prepare Failed");
			RETURN_FALSE;
		}

		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	}
}
/* }}} */

/* {{{ static param_node* build_list( stmt_res, param_no, data_type, precision, scale, nullable )
*/
static param_node* build_list( stmt_handle *stmt_res, int param_no, SQLSMALLINT data_type, SQLUINTEGER precision, SQLSMALLINT scale, SQLSMALLINT nullable )
{
	param_node *tmp_curr = NULL, *curr = stmt_res->head_cache_list, *prev = NULL;

	/* Allocate memory and make new node to be added */
	tmp_curr = (param_node *)emalloc(sizeof(param_node));
	/* assign values */
	tmp_curr->data_type = data_type;
	tmp_curr->param_size = precision;
	tmp_curr->nullable = nullable;
	tmp_curr->scale = scale;
	tmp_curr->param_num = param_no;
	tmp_curr->file_options = SQL_FILE_READ;
	tmp_curr->param_type = DB2_PARAM_IN;

	MAKE_STD_ZVAL(tmp_curr->value);
	while ( curr != NULL ) {
		prev = curr;
		curr = curr->next;
	}

	if (stmt_res->head_cache_list == NULL) {
		stmt_res->head_cache_list = tmp_curr;
	} else {
		prev->next = tmp_curr;
	}

	tmp_curr->next = curr;

	return tmp_curr;
}
/* }}} */

/* {{{ static int _php_db2_bind_data( stmt_handle *stmt_res, param_node *curr, zval **bind_data )
*/
static int _php_db2_bind_data( stmt_handle *stmt_res, param_node *curr, zval **bind_data)
{
	int rc;

	/* copy data over from bind_data */
	*(curr->value) = **bind_data;
	/*
		Hmm. Will this actually account for all types?
		Lets assume that db2_execute is being executed in a loop. The types
		contained in this might change. If we just do one MAKE_STD_VAL will
		it take care of all combinations on deep copies?
		For example, from an INT to STRING?
	*/
	zval_copy_ctor(curr->value);

	/* Have to use SQLBindFileToParam if PARAM is type DB2_PARAM_FILE */
	if ( curr->param_type == DB2_PARAM_FILE) {
		/* Only string types can be bound */
		if ( Z_TYPE_PP(bind_data) != IS_STRING) {
			return SQL_ERROR;
		}

		/* Bind file name string */
		rc = SQLBindFileToParam((SQLHSTMT)stmt_res->hstmt, curr->param_num,
			curr->data_type, ((curr->value)->value.str.val),
			(SQLSMALLINT *)((curr->value)->value.str.len), &(curr->file_options),
			Z_STRLEN_P(curr->value), 0);

		return 0;
	}

	switch(Z_TYPE_PP(bind_data)) {
		case IS_LONG:
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
				curr->param_type, SQL_C_LONG, curr->data_type, curr->param_size,
				curr->scale, &((curr->value)->value.lval), 0, NULL);
			break;

		/* Convert BOOLEAN types to LONG for DB2 / Cloudscape */
		case IS_BOOL:
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
				curr->param_type, SQL_C_LONG, curr->data_type, curr->param_size,
				curr->scale, &((curr->value)->value.lval), 0, NULL);
			break;

		case IS_DOUBLE:
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
				curr->param_type, SQL_C_DOUBLE, curr->data_type, curr->param_size,
				curr->scale, &((curr->value)->value.dval), 0, NULL);
			break;

		case IS_STRING:
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
				curr->param_type, SQL_C_CHAR, curr->data_type, curr->param_size,
				curr->scale, ((curr->value)->value.str.val),
				Z_STRLEN_P(curr->value), NULL);
			break;

		case IS_NULL:
			Z_LVAL_P(curr->value) = SQL_NULL_DATA;
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
				curr->param_type, SQL_C_DEFAULT, curr->data_type, curr->param_size,
				curr->scale, &(curr->value), 0, &((curr->value)->value.lval));
			break;

		default:
			return SQL_ERROR;
	}
	return rc;
}
/* }}} */

/* {{{ static int _php_db2_execute_helper(stmt_res, data, int bind_cmp_list TSRMLS_DC)
	*/
static int _php_db2_execute_helper(stmt_handle *stmt_res, zval **data, int bind_cmp_list, int bind_params TSRMLS_DC)
{
	int rc=SQL_SUCCESS;
	param_node *curr = NULL;	/* To traverse the list */
	zval **bind_data;			/* Data value from symbol table */

	/* Used in call to SQLDescribeParam if needed */
	int param_no;
	SQLSMALLINT data_type;
	SQLUINTEGER precision;
	SQLSMALLINT scale;
	SQLSMALLINT nullable;

	/* This variable means that we bind the complete list of params cached */
	/* The values used are fetched from the active symbol table */
	/* TODO: Enhance this part to check for stmt_res->file_param */
	/* If this flag is set, then use SQLBindParam, else use SQLExtendedBind */
	if ( bind_cmp_list ) {
		/* Bind the complete list sequentially */
		/* Used when no parameters array is passed in */
		curr = stmt_res->head_cache_list;

		while (curr != NULL ) {
			/* Fetch data from symbol table */
			if ( zend_hash_find(EG(active_symbol_table), curr->varname,
				strlen(curr->varname)+1, (void **) &bind_data ) != FAILURE ) {
				rc = _php_db2_bind_data( stmt_res, curr, bind_data);
				if ( rc == SQL_ERROR ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binding Error 1");
					return rc;
				}
				curr = curr->next;
			} else {
				/* value not found in the active symbol table */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Value Not Bound");
				return SQL_ERROR;
			}
		}
		return 0;
	} else {
		/* Bind only the data value passed in to the Current Node */
		if ( data != NULL ) {
			if ( bind_params ) {

				/*
					This condition applies if the parameter has not been
					bound using db2_bind_param. Need to describe the
					parameter and then bind it.
				*/
				
				param_no = ++stmt_res->num_params;

				rc = SQLDescribeParam((SQLHSTMT)stmt_res->hstmt, param_no,
					(SQLSMALLINT*)&data_type, &precision, (SQLSMALLINT*)&scale,
					(SQLSMALLINT*)&nullable);
				if ( rc == SQL_ERROR ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Describe Param Failed");
					return rc;
				}

				curr = build_list( stmt_res, param_no, data_type, precision, scale, nullable );

				rc = _php_db2_bind_data( stmt_res, curr, data);
				if ( rc == SQL_ERROR ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binding Error 2");
					return rc;
				}
			} else {
				/*
					This is always at least the head_cache_node -- assigned in
					db2_execute(), if params have been bound.
				*/
				curr = stmt_res->current_node;

				if ( curr != NULL ) {
					rc = _php_db2_bind_data( stmt_res, curr, data);
					if ( rc == SQL_ERROR ) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binding Error 2");
						return rc;
					}
					stmt_res->current_node = curr->next;
				}
			}
			return rc;
		}
	}
	return rc;
}
/* }}} */

/* {{{ proto bool db2_execute(resource stmt [, array parameters_array])
Executes a prepared SQL statement */
PHP_FUNCTION(db2_execute)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *parameters_array = NULL;
	stmt_handle *stmt_res;
	int rc, numOpts, i, bind_params = 0;
	SQLSMALLINT num;

	/* This is used to loop over the param cache */
	param_node *tmp_curr, *prev_ptr, *curr_ptr;

	zval **data;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|a", &stmt, &parameters_array) == FAILURE) {
		return;
	}

	/* Get values from symbol tables */
	/* Assign values into param nodes */
	/* Check types/conversions */
	/* Bind parameters */
	/* Execute */
	/* Return values back to symbol table for OUT params */

	if (stmt) {

		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		/* Free any cursors that might have been allocated in a previous call to SQLExecute */
		SQLFreeStmt((SQLHSTMT)stmt_res->hstmt, SQL_CLOSE);

		/* This ensures that each call to db2_execute start from scratch */
		stmt_res->current_node = stmt_res->head_cache_list;

		rc = SQLNumParams((SQLHSTMT)stmt_res->hstmt, (SQLSMALLINT*)&num);

		if ( num != 0 ) {
			/* Parameter Handling */
			if ( parameters_array != NULL ) {
				/* Make sure db2_bind_param has been called */
				/* If the param list is NULL -- ERROR */
				if ( stmt_res->head_cache_list == NULL ) {
					bind_params = 1;
				}

				numOpts = zend_hash_num_elements(Z_ARRVAL_P(parameters_array));
				if (numOpts > num) {
					/* More are passed in -- Warning - Use the max number present */
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Param count incorrect");
					numOpts = stmt_res->num_params;
				} else if (numOpts < num) {
					/* If there are less params passed in, than are present -- Error */
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Param count incorrect");
					RETURN_FALSE;
				}

				zend_hash_internal_pointer_reset(Z_ARRVAL_P(parameters_array));
				for ( i = 0; i < numOpts; i++) {
					/* Bind values from the parameters_array to params */
					zend_hash_get_current_data(Z_ARRVAL_P(parameters_array), (void**)&data);

					/*
						The 0 denotes that you work only with the current node.
						The 4th argument specifies whether the data passed in
						has been described. So we need to call SQLDescribeParam
						before binding depending on this.
					*/
					rc = _php_db2_execute_helper(stmt_res, data, 0, bind_params TSRMLS_CC);
					if ( rc == SQL_ERROR) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binding Error");
						RETURN_FALSE;
					}

					/* Move array ptr forward */
					zend_hash_move_forward(Z_ARRVAL_P(parameters_array));
				}
			} else {
				/* No additional params passed in. Use values already bound. */
				if ( num > stmt_res->num_params ) {
					/* More parameters than we expected */
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "More parameters bound than present");
				} else if ( num < stmt_res->num_params ) {
					/* Fewer parameters than we expected */
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Less parameters bound than present");
					RETURN_FALSE;
				}

				/* Param cache node list is empty -- No params bound */
				if ( stmt_res->head_cache_list == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameters not bound");
					RETURN_FALSE;
				} else {
					/* The 1 denotes that you work with the whole list */
					/* And bind sequentially */
					rc = _php_db2_execute_helper(stmt_res, NULL, 1, 0 TSRMLS_CC);
					if ( rc == SQL_ERROR ) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binding Error 3");
						RETURN_FALSE;
					}
				}
			}
		} else {
			/* No Parameters */
			/* We just execute the statement. No additional work needed. */
			rc = SQLExecute((SQLHSTMT)stmt_res->hstmt);
			if ( rc == SQL_ERROR ) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
				RETURN_FALSE;
			}
			RETURN_TRUE;
		}

		/* Execute Stmt -- All parameters bound */
		rc = SQLExecute((SQLHSTMT)stmt_res->hstmt);
		if ( rc == SQL_ERROR ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
			RETVAL_FALSE;
		}

		/* cleanup dynamic bindings if present */
		if ( bind_params == 1 ) {
			/* Free param cache list */
			curr_ptr = stmt_res->head_cache_list;
			prev_ptr = stmt_res->head_cache_list;

			while (curr_ptr != NULL) {
				curr_ptr = curr_ptr->next;

				/* Free Values */
				if ( Z_TYPE_P(prev_ptr->value) == IS_STRING ) {
					efree((prev_ptr->value)->value.str.val);
				}

				efree(prev_ptr->value);
				efree(prev_ptr);

				prev_ptr = curr_ptr;
			}

			stmt_res->head_cache_list = NULL;
			stmt_res->num_params = 0;
		} else {
			/* Bind the IN/OUT Params back into the active symbol table */
			tmp_curr = stmt_res->head_cache_list;
			while (tmp_curr != NULL) {
				switch(tmp_curr->param_type) {
					case DB2_PARAM_OUT:
					case DB2_PARAM_INOUT:
						/* cant use zend_hash_update because the symbol need not exist. It might need to be created */
						ZEND_SET_SYMBOL(EG(active_symbol_table), tmp_curr->varname, tmp_curr->value);

					default:
						break;
				}
				tmp_curr = tmp_curr->next;
			}
		}

		if ( rc != SQL_ERROR ) {
			RETURN_TRUE;
		}
	}
}
/* }}} */

/* {{{ proto string db2_conn_errormsg([resource connection])
Returns a string containing the last connection error message */
PHP_FUNCTION(db2_conn_errormsg)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	char* return_str = NULL;	/* This variable is used by _php_db2_check_sql_errors to return err strings */

	if (zend_parse_parameters(argc TSRMLS_CC, "|r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		return_str = (char*)emalloc(DB2_MAX_ERR_MSG_LEN);
		memset(return_str, 0, DB2_MAX_ERR_MSG_LEN);

		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, -1, 0, return_str, DB2_ERRMSG, conn_res->errormsg_recno_tracker TSRMLS_CC);
		if(conn_res->errormsg_recno_tracker - conn_res->error_recno_tracker >= 1)
			conn_res->error_recno_tracker = conn_res->errormsg_recno_tracker;
		conn_res->errormsg_recno_tracker++;

		RETURN_STRING(return_str, 0);
	} else {
		RETURN_STRING(IBM_DB2_G(__php_conn_err_msg), 1);
	}
}
/* }}} */

/* {{{ proto string db2_stmt_errormsg([resource stmt])
Returns a string containing the last SQL statement error message */
PHP_FUNCTION(db2_stmt_errormsg)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res;
	char* return_str = NULL; /* This variable is used by _php_db2_check_sql_errors to return err strings */

	if (zend_parse_parameters(argc TSRMLS_CC, "|r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		return_str = (char*)emalloc(DB2_MAX_ERR_MSG_LEN);
		memset(return_str, 0, DB2_MAX_ERR_MSG_LEN);

		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, -1, 0, return_str, DB2_ERRMSG, stmt_res->errormsg_recno_tracker TSRMLS_CC);
		if(stmt_res->errormsg_recno_tracker - stmt_res->error_recno_tracker >= 1)
			stmt_res->error_recno_tracker = stmt_res->errormsg_recno_tracker;
		stmt_res->errormsg_recno_tracker++;

		RETURN_STRING(return_str, 0);
	} else {
		RETURN_STRING(IBM_DB2_G(__php_stmt_err_msg), 1);
	}
}
/* }}} */

/* {{{ proto string db2_conn_error([resource connection])
Returns a string containing the SQLSTATE returned by the last connection attempt */
PHP_FUNCTION(db2_conn_error)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;

	char *return_str = NULL; /* This variable is used by _php_db2_check_sql_errors to return err strings */
	if (zend_parse_parameters(argc TSRMLS_CC, "|r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		return_str = (char*)emalloc(SQL_SQLSTATE_SIZE + 1);
		memset(return_str, 0, SQL_SQLSTATE_SIZE + 1);

		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, -1, 0, return_str, DB2_ERR, conn_res->error_recno_tracker TSRMLS_CC);
		if (conn_res->error_recno_tracker - conn_res->errormsg_recno_tracker >= 1) {
			conn_res->errormsg_recno_tracker = conn_res->error_recno_tracker;
		}
		conn_res->error_recno_tracker++;

		RETURN_STRING(return_str, 0);
	} else {
		RETURN_STRING(IBM_DB2_G(__php_conn_err_state), 1);
	}
}
/* }}} */

/* {{{ proto string db2_stmt_error([resource stmt])
Returns a string containing the SQLSTATE returned by an SQL statement */
PHP_FUNCTION(db2_stmt_error)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res;
	char* return_str = NULL; /* This variable is used by _php_db2_check_sql_errors to return err strings */

	if (zend_parse_parameters(argc TSRMLS_CC, "|r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		return_str = (char*)emalloc(SQL_SQLSTATE_SIZE + 1);
		memset(return_str, 0, SQL_SQLSTATE_SIZE + 1);

		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, -1, 0, return_str, DB2_ERR, stmt_res->error_recno_tracker TSRMLS_CC);

		if (stmt_res->error_recno_tracker - stmt_res->errormsg_recno_tracker >= 1) {
			stmt_res->errormsg_recno_tracker = stmt_res->error_recno_tracker;
		}
		stmt_res->error_recno_tracker++;

		RETURN_STRING(return_str, 0);
	} else {
		RETURN_STRING(IBM_DB2_G(__php_stmt_err_state), 1);
	}
}
/* }}} */

/* {{{ proto resource db2_next_result(resource stmt)
Requests the next result set from a stored procedure */
PHP_FUNCTION(db2_next_result)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res, *new_stmt_res=NULL;
	int rc = 0;
	SQLHANDLE new_hstmt;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		_php_db2_clear_stmt_err_cache(TSRMLS_C);

		/* alloc handle and return only if it errors */
		rc = SQLAllocHandle(SQL_HANDLE_STMT, stmt_res->hdbc, &new_hstmt);
		if ( rc < SQL_SUCCESS ) {
			_php_db2_check_sql_errors(stmt_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
		}
		rc = SQLNextResult((SQLHSTMT)stmt_res->hstmt, (SQLHSTMT)new_hstmt);
		if( rc != SQL_SUCCESS ) {
			if(rc < SQL_SUCCESS) {
				_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			}
			SQLFreeHandle(SQL_HANDLE_STMT, new_hstmt);
			RETURN_FALSE;
		}

		/* Initialize stmt resource members with default values. */
		/* Parsing will update options if needed */
		new_stmt_res = (stmt_handle *)emalloc(sizeof(stmt_handle));
		new_stmt_res->s_bin_mode = stmt_res->s_bin_mode;
		new_stmt_res->cursor_type = stmt_res->cursor_type;
		new_stmt_res->head_cache_list = NULL;
		new_stmt_res->current_node = NULL;
		new_stmt_res->num_params = 0;
		new_stmt_res->file_param = 0;
		new_stmt_res->column_info = NULL;
		new_stmt_res->num_columns = 0;
		new_stmt_res->row_data = NULL;
		new_stmt_res->hstmt = new_hstmt;
		new_stmt_res->hdbc = stmt_res->hdbc;

		ZEND_REGISTER_RESOURCE(return_value, new_stmt_res, le_stmt_struct);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int db2_num_fields(resource stmt)
Returns the number of fields contained in a result set */
PHP_FUNCTION(db2_num_fields)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res;
	int rc = 0;
	SQLSMALLINT indx = 0;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		rc = SQLNumResultCols((SQLHSTMT)stmt_res->hstmt, &indx);
		if ( rc == SQL_ERROR ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "SQLNumResultCols failed");
			RETURN_FALSE;
		}

		RETURN_LONG(indx);
	}
}
/* }}} */

/* {{{ proto int db2_num_rows(resource stmt)
Returns the number of rows affected by an SQL statement */
PHP_FUNCTION(db2_num_rows)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res;
	int rc = 0;
	SQLINTEGER count = 0;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);


		rc = SQLRowCount((SQLHSTMT)stmt_res->hstmt, &count);
		if ( rc == SQL_ERROR ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "SQLRowCount failed");
			RETURN_FALSE;
		}

		RETURN_LONG(count);
	}
}
/* }}} */

/* {{{ static int _php_db2_get_column_by_name(stmt_handle *stmt_res, char *col_name, int col)
	*/
static int _php_db2_get_column_by_name(stmt_handle *stmt_res, char *col_name, int col)
{
	int i;
	/* get column header info*/
	if ( stmt_res->column_info == NULL ) {
		if (_php_db2_get_result_set_info(stmt_res)<0) {
			return -1;
		}
	}
	if ( col_name == NULL ) {
		if ( col >= 0 && col < stmt_res->num_columns) {
			return col;
		} else {
			return -1;
		}
	}
	/* should start from 0 */
	i=0;
	while (i < stmt_res->num_columns) {
		if (strcmp(stmt_res->column_info[i].name,col_name) == 0) {
			return i;
		}
		i++;
	}
	return -1;
}
/* }}} */

/* {{{ proto string db2_field_name(resource stmt, mixed column)
Returns the name of the column in the result set */
PHP_FUNCTION(db2_field_name)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	stmt_handle* stmt_res = NULL;
	char *col_name = NULL;
	int col = -1;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	RETURN_STRING(stmt_res->column_info[col].name,1);
}
/* }}} */

/* {{{ proto long db2_field_display_size(resource stmt, mixed column)
Returns the maximum number of bytes required to display a column */
PHP_FUNCTION(db2_field_display_size)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	int col =- 1;
	char *col_name = NULL;
	stmt_handle *stmt_res = NULL;
	int rc;
	SQLINTEGER colDataDisplaySize;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		RETURN_FALSE;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	rc = SQLColAttribute((SQLHSTMT)stmt_res->hstmt,(SQLSMALLINT)col+1,
			SQL_DESC_DISPLAY_SIZE,NULL,0, NULL,&colDataDisplaySize);
	if ( rc < SQL_SUCCESS ) {
		RETURN_FALSE;
	}
	RETURN_LONG(colDataDisplaySize);
}
/* }}} */

/* {{{ proto long db2_field_num(resource stmt, mixed column)
Returns the position of the named column in a result set */
PHP_FUNCTION(db2_field_num)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	stmt_handle* stmt_res = NULL;
	char *col_name = NULL;
	int col = -1;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	RETURN_LONG(col);
}
/* }}} */

/* {{{ proto long db2_field_precision(resource stmt, mixed column)
Returns the precision for the indicated column in a result set */
PHP_FUNCTION(db2_field_precision)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	stmt_handle* stmt_res = NULL;
	char *col_name = NULL;
	int col = -1;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	RETURN_LONG(stmt_res->column_info[col].size);

}
/* }}} */

/* {{{ proto long db2_field_scale(resource stmt, mixed column)
Returns the scale of the indicated column in a result set */
PHP_FUNCTION(db2_field_scale)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	stmt_handle* stmt_res = NULL;
	char *col_name = NULL;
	int col = -1;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	RETURN_LONG(stmt_res->column_info[col].scale);
}
/* }}} */

/* {{{ proto string db2_field_type(resource stmt, mixed column)
Returns the data type of the indicated column in a result set */
PHP_FUNCTION(db2_field_type)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	stmt_handle* stmt_res = NULL;
	char *col_name = NULL;
	char *str_val = "";
	int col = -1;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	switch (stmt_res->column_info[col].type) {
		case SQL_SMALLINT:
		case SQL_INTEGER:
		case SQL_BIGINT:
			str_val = "int";
			break;
		case SQL_REAL:
		case SQL_FLOAT:
		case SQL_DOUBLE:
		case SQL_DECIMAL:
		case SQL_NUMERIC:
			str_val = "real";
			break;
		case SQL_CLOB:
		case SQL_BLOB:
			str_val = "blob";
			break;
		default:
			str_val = "string";
			break;
	}
	RETURN_STRING(str_val,1);
}
/* }}} */

/* {{{ proto long db2_field_width(resource stmt, mixed column)
Returns the width of the current value of the indicated column in a result set */
PHP_FUNCTION(db2_field_width)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	int col=-1;
	char *col_name = NULL;
	stmt_handle *stmt_res = NULL;
	int rc;
	SQLINTEGER colDataSize;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		RETURN_FALSE;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}
	if ( Z_TYPE_P(column)==IS_LONG ) {
		col = Z_LVAL_P(column);
	} else {
		col_name = Z_STRVAL_P(column);
	}
	col = _php_db2_get_column_by_name(stmt_res,col_name, col);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	rc = SQLColAttribute((SQLHSTMT)stmt_res->hstmt,(SQLSMALLINT)col+1,
			SQL_DESC_LENGTH,NULL,0, NULL,&colDataSize);
	if ( rc != SQL_SUCCESS ) {
		RETURN_FALSE;
	}
	RETURN_LONG(colDataSize);
}
/* }}} */

/* {{{ proto long db2_cursor_type(resource stmt)
Returns the cursor type used by the indicated statement resource */
PHP_FUNCTION(db2_cursor_type)
{
	int argc = ZEND_NUM_ARGS();
	zval *stmt = NULL;
	stmt_handle *stmt_res = NULL;
	int stmt_id = -1;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &stmt) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}

	RETURN_LONG(stmt_res->cursor_type != DB2_FORWARD_ONLY);
}
/* }}} */

/* {{{ proto bool db2_rollback(resource connection)
Rolls back a transaction */
PHP_FUNCTION(db2_rollback)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		rc = SQLEndTran(SQL_HANDLE_DBC, conn_res->hdbc, SQL_ROLLBACK);

		if ( rc == SQL_ERROR ) {
			RETVAL_FALSE;
			return;
		} else {
			RETURN_TRUE;
		}
	}
}
/* }}} */

/* {{{ proto bool db2_free_stmt(resource stmt)
Frees resources associated with the indicated statement resource */
PHP_FUNCTION(db2_free_stmt)
{
	int argc = ZEND_NUM_ARGS();
	zval *stmt = NULL;
	int stmt_id = -1, rc = 0;
	stmt_handle *stmt_res;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &stmt) == FAILURE) {
		RETURN_FALSE;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		rc = SQLFreeStmt((SQLHSTMT)stmt_res->hstmt, SQL_CLOSE);
		rc = SQLFreeHandle( SQL_HANDLE_STMT, stmt_res->hstmt);

		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ static RETCODE _php_db2_get_data(stmt_handle *stmt_res, int col_num, short ctype, void *buff, int in_length, SQLINTEGER *out_length) */
static RETCODE _php_db2_get_data(stmt_handle *stmt_res, int col_num, short ctype, void *buff, int in_length, SQLINTEGER *out_length)
{
	RETCODE rc=SQL_SUCCESS;

	rc = SQLGetData((SQLHSTMT)stmt_res->hstmt, col_num, ctype, buff, in_length, out_length);
	return rc;
}
/* }}} */

/* {{{ proto mixed db2_result(resource stmt, mixed column)
Returns a single column from a row in the result set */
PHP_FUNCTION(db2_result)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	zval *column = NULL;
	stmt_handle *stmt_res;
	long col_num;
	RETCODE rc;
	void	*out_ptr;
	char	*out_char_ptr;
	SQLINTEGER in_length, out_length=-10; /*Initialize out_length to some meaningless value*/
	SQLSMALLINT column_type, lob_bind_type= SQL_C_BINARY;
	double double_val;
	long long_val;

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		if(Z_TYPE_P(column) == IS_STRING) {
			col_num = _php_db2_get_column_by_name(stmt_res, Z_STRVAL_P(column), -1);
		} else {
			col_num = Z_LVAL_P(column);
		}

		/* get column header info*/
		if ( stmt_res->column_info == NULL ) {
			if (_php_db2_get_result_set_info(stmt_res)<0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column information cannot be retrieved");
				RETURN_FALSE;
			}
		}

		if(col_num < 0 || col_num >= stmt_res->num_columns) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column ordinal out of range");
		}

		/* get the data */
		column_type = stmt_res->column_info[col_num].type;
		switch(column_type) {
			case SQL_CHAR:
			case SQL_VARCHAR:
			case SQL_LONGVARCHAR:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
			case SQL_TYPE_TIMESTAMP:
			case SQL_BIGINT:
			case SQL_DECIMAL:
			case SQL_NUMERIC:
				in_length = stmt_res->column_info[col_num].size+1;
				out_ptr = (SQLPOINTER)emalloc(in_length);
				if ( out_ptr == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory");
					RETURN_FALSE;
				}
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_CHAR, out_ptr, in_length, &out_length);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				} else {
					RETVAL_STRING((char*)out_ptr, 1);
					efree(out_ptr);
				}
				break;

			case SQL_SMALLINT:
			case SQL_INTEGER:
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_SLONG, &long_val, sizeof(long_val), &out_length);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				} else {
					RETURN_LONG(long_val);
				}
				break;

			case SQL_REAL:
			case SQL_FLOAT:
			case SQL_DOUBLE:
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_DOUBLE, &double_val, sizeof(double_val), &out_length);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				} else {
					RETURN_DOUBLE(double_val);
				}
				break;

			case SQL_CLOB:
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_CHAR, NULL, 0, (SQLINTEGER *)&in_length);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}
				out_char_ptr = (char*)emalloc(in_length+1);
				if ( out_char_ptr == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
					RETURN_FALSE;
				}
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_CHAR, (void*)out_char_ptr, in_length+1, &out_length);
				if (rc == SQL_ERROR) {
					RETURN_FALSE;
				}

				out_char_ptr[in_length] = '\0';
				RETURN_STRING(out_char_ptr, 0);
				break;

			case SQL_BLOB:
			case SQL_BINARY:
			case SQL_LONGVARBINARY:
			case SQL_VARBINARY:
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_BINARY, NULL, 0, (SQLINTEGER *)&in_length);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (in_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}

				switch (stmt_res->s_bin_mode) {
					case DB2_PASSTHRU:
						RETVAL_EMPTY_STRING();
						break;
						/* returns here */
					case DB2_CONVERT:
						in_length *= 2;
						lob_bind_type = SQL_C_CHAR;
						/* fall-through */

					case DB2_BINARY:

						out_ptr = (SQLPOINTER)emalloc(in_length);
						if ( out_ptr == NULL ) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
							RETURN_FALSE;
						}
						rc = _php_db2_get_data(stmt_res, col_num+1, lob_bind_type, out_ptr, in_length, &out_length);
						if (rc == SQL_ERROR) {
							RETURN_FALSE;
						}
						RETVAL_STRINGL((char*)out_ptr,out_length, 0);
					default:
						break;
				}
				break;
			default:
				break;
		}
	} else {
		/* throw error? */
		/* do the same in all APIs*/
	}
}
/* }}} */

/* {{{ static void _php_db2_bind_fetch_helper(INTERNAL_FUNCTION_PARAMETERS, int op)
*/
static void _php_db2_bind_fetch_helper(INTERNAL_FUNCTION_PARAMETERS, int op)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1, rc = -1, i;
	long row_number=-1;
	zval *stmt = NULL;
	stmt_handle *stmt_res = NULL;
	SQLSMALLINT column_type, lob_bind_type = SQL_C_BINARY;
	db2_row_data_type *row_data;
	SQLINTEGER out_length, tmp_length;
	unsigned char *out_ptr;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|l", &stmt, &row_number) == FAILURE) {
		return;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}

	_php_db2_init_error_info(stmt_res);

	/* get column header info*/
	if ( stmt_res->column_info == NULL ) {
		if (_php_db2_get_result_set_info(stmt_res)<0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column information cannot be retrieved");
			RETURN_FALSE;
		}
	}
	/* bind the data */
	if ( stmt_res->row_data == NULL ) {
		rc = _php_db2_bind_column_helper(stmt_res);
		if ( rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column binding cannot be done");
			RETURN_FALSE;
		}
	}
	/* check if row_number is present */
	if ( argc == 2 ) {
		if ( row_number <= 0 ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Fetch type out of range");
			RETURN_FALSE;
		} else {
			rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_ABSOLUTE, row_number);
		}
	} else {
		rc = SQLFetch((SQLHSTMT)stmt_res->hstmt);
	}

	if (rc == SQL_NO_DATA_FOUND) {
		RETURN_FALSE;
	} else if ( rc != SQL_SUCCESS ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Fetch Failure");
		RETURN_FALSE;
	}
	/* copy the data over return_value */
	array_init(return_value);
	for (i=0; i<stmt_res->num_columns; i++) {
		column_type = stmt_res->column_info[i].type;
		row_data = &stmt_res->row_data[i].data;
		out_length = stmt_res->row_data[i].out_length;
		if (out_length == SQL_NULL_DATA) {
			if ( op & DB2_FETCH_ASSOC ) {
				add_assoc_null(return_value, stmt_res->column_info[i].name);
			}
			if ( op & DB2_FETCH_INDEX ) {
				add_index_null(return_value, i);
			}
		} else {
			switch(column_type) {
				case SQL_CHAR:
				case SQL_VARCHAR:
				case SQL_LONGVARCHAR:
				case SQL_TYPE_DATE:
				case SQL_TYPE_TIME:
				case SQL_TYPE_TIMESTAMP:
				case SQL_BIGINT:
				case SQL_DECIMAL:
				case SQL_NUMERIC:

					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_stringl(return_value, stmt_res->column_info[i].name,
							row_data->str_val, strlen(row_data->str_val), 1);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_stringl(return_value, i, row_data->str_val,
							strlen(row_data->str_val), 1);
					}
					break;
				case SQL_SMALLINT:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_long(return_value, stmt_res->column_info[i].name, row_data->s_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_long(return_value, i, row_data->s_val);
					}
					break;
				case SQL_INTEGER:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_long(return_value, stmt_res->column_info[i].name,
							row_data->i_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_long(return_value, i, row_data->i_val);
					}
					break;

				case SQL_REAL:
				case SQL_FLOAT:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_double(return_value, stmt_res->column_info[i].name, row_data->f_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_double(return_value, i, row_data->f_val);
					}
					break;

				case SQL_DOUBLE:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_double(return_value, stmt_res->column_info[i].name, row_data->d_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_double(return_value, i, row_data->d_val);
					}
					break;

				case SQL_BINARY:
				case SQL_LONGVARBINARY:
				case SQL_VARBINARY:
					if ( stmt_res->s_bin_mode == DB2_PASSTHRU ) {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, stmt_res->column_info[i].name, "", 0, 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, "", 0, 1);
						}
					} else {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, stmt_res->column_info[i].name,
								row_data->str_val, strlen(row_data->str_val), 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, row_data->str_val,
								strlen(row_data->str_val), 1);
						}
					}
					break;

				case SQL_BLOB:
					out_ptr = NULL;
					rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY, NULL, 0, (SQLINTEGER *)&tmp_length);
					if ( rc == SQL_ERROR ) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Determine LOB Size");
							RETURN_FALSE;
					}

					if (tmp_length == SQL_NULL_DATA) {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_null(return_value, stmt_res->column_info[i].name);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_null(return_value, i);
						}
					} else {
						switch (stmt_res->s_bin_mode) {
							case DB2_PASSTHRU:
								if ( op & DB2_FETCH_ASSOC ) {
										add_assoc_null(return_value, stmt_res->column_info[i].name);
								}
								if ( op & DB2_FETCH_INDEX ) {
										add_index_null(return_value, i);
								}	
								break;

							case DB2_CONVERT:
								tmp_length = 2*tmp_length + 1;
								lob_bind_type = SQL_C_CHAR;
								/* fall-through */

							case DB2_BINARY:
								out_ptr = (SQLPOINTER)emalloc(tmp_length);
								memset(out_ptr, 0, tmp_length);
	
								if ( out_ptr == NULL ) {
									php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
									RETURN_FALSE;
								}
								rc = _php_db2_get_data(stmt_res, i+1, lob_bind_type, out_ptr, tmp_length, &out_length);
								if (rc == SQL_ERROR) {
									RETURN_FALSE;
								}
	
								if ( op & DB2_FETCH_ASSOC ) {
									add_assoc_stringl(return_value, stmt_res->column_info[i].name, out_ptr, out_length, 0);
								}
								if ( op & DB2_FETCH_INDEX ) {
									add_index_stringl(return_value, i, out_ptr, out_length, DB2_FETCH_BOTH & op);
								}
								break;
							default:
								break;
						}
					}
					break;

				case SQL_CLOB:
					out_ptr = NULL;
					rc = _php_db2_get_data(stmt_res, i+1, SQL_C_CHAR, NULL, 0, (SQLINTEGER *)&tmp_length);
					if ( rc == SQL_ERROR ) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Determine LOB Size");
							RETURN_FALSE;
					}

					if (tmp_length == SQL_NULL_DATA) {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_null(return_value, stmt_res->column_info[i].name);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_null(return_value, i);
						}
					} else {
						out_ptr = (SQLPOINTER)emalloc(tmp_length+1);
						memset(out_ptr, 0, tmp_length+1);

						if ( out_ptr == NULL ) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
							RETURN_FALSE;
						}

						rc = _php_db2_get_data(stmt_res, i+1, SQL_C_CHAR, out_ptr, tmp_length+1, &out_length);
						if (rc == SQL_ERROR) {
							RETURN_FALSE;
						}
						out_ptr[tmp_length] = '\0';

						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, stmt_res->column_info[i].name, out_ptr, tmp_length+1, 0);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, out_ptr, tmp_length+1, DB2_FETCH_BOTH & op);
						}
					}
					break;

				default:
					break;
			}
		}
	}
}
/* }}} */

/* {{{ proto bool db2_fetch_row(resource stmt [, int row_number])
Sets the fetch pointer to the next or requested row in a result set */
PHP_FUNCTION(db2_fetch_row)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	long row_number;
	zval *stmt = NULL;
	stmt_handle* stmt_res = NULL;
	int rc;

	if (zend_parse_parameters(argc TSRMLS_CC, "r|l", &stmt, &row_number) == FAILURE) {
		return;
	}

	if (stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}

	/*check if row_number is present*/
	if ( argc == 2 ) {
		if ( row_number <= 0 ) {
			RETURN_FALSE;
		} else {
			rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_ABSOLUTE, row_number );
		}
	} else {
		rc = SQLFetch((SQLHSTMT)stmt_res->hstmt);
	}
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array db2_fetch_assoc(resource stmt [, int row_number])
Returns an array, indexed by column name, representing a row in a result set */
PHP_FUNCTION(db2_fetch_assoc)
{
	_php_db2_bind_fetch_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, DB2_FETCH_ASSOC);
}
/* }}} */

/* {{{ proto object db2_fetch_object(resource stmt [, int row_number])
Returns an object with properties that correspond to the fetched row */
PHP_FUNCTION(db2_fetch_object)
{
	_php_db2_bind_fetch_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, DB2_FETCH_ASSOC);

	if (Z_TYPE_P(return_value) == IS_ARRAY) {
		object_and_properties_init(return_value, ZEND_STANDARD_CLASS_DEF_PTR, Z_ARRVAL_P(return_value));
	}
}
/* }}} */

/* {{{ proto array db2_fetch_array(resource stmt [, int row_number])
Returns an array, indexed by column position, representing a row in a result set */
PHP_FUNCTION(db2_fetch_array)
{
	_php_db2_bind_fetch_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, DB2_FETCH_INDEX);
}
/* }}} */

/* {{{ proto array db2_fetch_both(resource stmt [, int row_number])
Returns an array, indexed by both column name and position, representing a row in a result set */
PHP_FUNCTION(db2_fetch_both)
{
	_php_db2_bind_fetch_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, DB2_FETCH_BOTH);
}
/* }}} */

/* {{{ proto bool db2_set_option(resource stmt, array options, int type)
Sets the specified option in the resource. TYPE field specifies the resource type (1 = Connection) */
PHP_FUNCTION(db2_set_option)
{
	int argc = ZEND_NUM_ARGS();
	int id = -1;
	zval *resc = NULL;
	zval *options;
	stmt_handle *stmt_res;
	conn_handle *conn_res;
	int rc = 0;
	long type;

	if (zend_parse_parameters(argc TSRMLS_CC, "ral", &resc, &options, &type) == FAILURE) {
		return;
	}

	if (resc) {
		if ( type == 1 ) {
			ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &resc, id, "Connection Resource", le_conn_struct, le_pconn_struct);

			rc = _php_db2_parse_options( options, SQL_HANDLE_DBC, conn_res TSRMLS_CC );
			if (rc == SQL_ERROR) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Options Array must have string indexes");
				RETURN_FALSE;
			}
		} else {
			ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &resc, id, "Statement Resource", le_stmt_struct);

			rc = _php_db2_parse_options( options, SQL_HANDLE_STMT, stmt_res TSRMLS_CC );
			if (rc == SQL_ERROR) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Options Array must have string indexes");
				RETURN_FALSE;
			}
		}

		RETURN_TRUE;
	} else {
		RETURN_FALSE;
    }
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
