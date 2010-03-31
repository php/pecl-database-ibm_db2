/*
  +----------------------------------------------------------------------+
  | Copyright IBM Corporation 2005-2008                                  |
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
  |          Dan Scott, Helmut Tessarek, Kellen Bombardier,              |
  |          Tony Cairns, Krishna Raman, Ambrish Bhargava                |
  +----------------------------------------------------------------------+

  $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "../ext/standard/info.h"
#include "../ext/standard/php_string.h"
#include "php_ibm_db2.h"

ZEND_DECLARE_MODULE_GLOBALS(ibm_db2)

#if ZEND_MODULE_API_NO > 20020429
#define ONUPDATEFUNCTION OnUpdateLong
#else
#define ONUPDATEFUNCTION OnUpdateInt
#endif

/* True global resources - no need for thread safety here */
static int le_conn_struct, le_stmt_struct, le_pconn_struct;

static void _php_db2_check_sql_errors( SQLHANDLE handle, SQLSMALLINT hType, int rc, int cpy_to_global, char* ret_str, int API, SQLSMALLINT recno TSRMLS_DC);
static void _php_db2_assign_options( void* handle, int type, char* opt_key, zval **data TSRMLS_DC );
static int _php_db2_parse_options( zval* options, int type, void* handle TSRMLS_DC );
static void _php_db2_clear_conn_err_cache(TSRMLS_D);
static void _php_db2_clear_stmt_err_cache(TSRMLS_D);
static void _php_db2_set_decfloat_rounding_mode_client(void* handle TSRMLS_DC);
static char * _php_db2_instance_name;
static int is_ios, is_zos;		/* 1 == TRUE; 0 == FALSE; */
#ifdef PASE 
/* Once SQL server mode has been set,
 * all SQL connections and SQL statements will
 * run in server mode.
 */
static int is_i5_server_mode;		/* 1 == TRUE; 0 == FALSE; */

/* i5/OS V6R1 introduced incompatible change at the compile level
 * adding from v6r1 sqlcli.h to allow one binary for
 * v5r3, v5r4, v6r1+
 */
static int is_i5os_classic;             /* 1 == v5r4-; 0 == v6r1+; */

#define SQL_ATTR_INFO_USERID         10103
#define SQL_ATTR_INFO_WRKSTNNAME     10104
#define SQL_ATTR_INFO_APPLNAME       10105
#define SQL_ATTR_INFO_ACCTSTR        10106
#endif /* PASE */

/* Defines a linked list structure for caching param data */
typedef struct _param_cache_node {
	SQLSMALLINT	data_type;			/* Datatype */
	SQLUINTEGER	param_size;			/* param size */
	SQLSMALLINT nullable;			/* is Nullable */
	SQLSMALLINT	scale;				/* Decimal scale */
#ifdef PASE /* SQLINTEGER vs. SQLUINTEGER */
	SQLINTEGER file_options;		/* File options if DB2_PARAM_FILE */
#else
	SQLUINTEGER file_options;		/* File options if DB2_PARAM_FILE */
#endif
	SQLINTEGER	bind_indicator;		/* indicator variable for SQLBindParameter */
	SQLINTEGER	long_value;			/* Value if this is an SQL_C_LONG type */
	SQLSMALLINT	short_strlen;		/* Length of string if this is an SQL_C_STRING type */
	int			param_num;			/* param number in stmt */
	int			param_type;			/* Type of param - INP/OUT/INP-OUT/FILE */
	char		*varname;			/* bound variable name */
	zval		*value;				/* Temp storage value */
	struct _param_cache_node *next;	/* Pointer to next node */
} param_node;

typedef struct _conn_handle_struct {
	SQLHANDLE henv;
	SQLHANDLE hdbc;
	long auto_commit;
	long c_bin_mode;
	long c_case_mode;
	long c_cursor_type;
#ifdef PASE /* i5 override php.ini */
	long c_i5_allow_commit;
	long c_i5_dbcs_alloc;
	long c_i5_sys_naming;
	char * c_i5_pending_cmd;
#endif /* PASE */
	int handle_active;
	SQLSMALLINT error_recno_tracker;
	SQLSMALLINT errormsg_recno_tracker;
	int flag_pconnect; /* Indicates that this connection is persistent */
} conn_handle;

typedef union {
	SQLINTEGER i_val;
	SQLDOUBLE d_val;
	SQLFLOAT f_val;
	SQLREAL r_val;
	SQLSMALLINT s_val;
	SQLCHAR *str_val;
} db2_row_data_type;

typedef struct {
	SQLINTEGER out_length;
	db2_row_data_type data;
} db2_row_type;

typedef struct _db2_result_set_info_struct {
	SQLCHAR		*name;
	SQLSMALLINT type;
#ifdef PASE /* SQLINTEGER vs. SQLUINTEGER */
	SQLINTEGER size;
#else
	SQLUINTEGER size;
#endif
	SQLSMALLINT scale;
	SQLSMALLINT nullable;
	SQLINTEGER lob_loc;
	SQLINTEGER loc_ind;
	SQLSMALLINT loc_type;
} db2_result_set_info;

typedef struct _stmt_handle_struct {
	SQLHANDLE hdbc;
	SQLHANDLE hstmt;
	long s_bin_mode;
	long cursor_type;
	long s_case_mode;
	long s_rowcount;
#ifdef PASE /* i5 override php.ini */
	long s_i5_allow_commit;
	long s_i5_dbcs_alloc;
	long s_i5_sys_naming;
#endif /* PASE */
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
#ifdef PASE /* db2_pclose - last ditch pconnect close */
	PHP_FE(db2_pclose,	NULL)
#endif /* PASE */
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
	PHP_FE(db2_server_info,	NULL)
	PHP_FE(db2_client_info,	NULL)
	PHP_FE(db2_escape_string,	NULL)
	PHP_FE(db2_lob_read,	NULL)
	PHP_FE(db2_get_option,	NULL)
	PHP_FALIAS(db2_getoption, db2_get_option,   NULL)
	PHP_FE(db2_last_insert_id,	NULL)
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
	PHP_IBM_DB2_VERSION, /* Replace with version number for your extension */
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
#ifdef PASE /* i5/OS ease of use turn off commit */
	STD_PHP_INI_BOOLEAN("ibm_db2.i5_allow_commit", "0", PHP_INI_SYSTEM, OnUpdateLong,
		i5_allow_commit, zend_ibm_db2_globals, ibm_db2_globals)
	STD_PHP_INI_BOOLEAN("ibm_db2.i5_dbcs_alloc", "0", PHP_INI_SYSTEM, OnUpdateLong,
		i5_dbcs_alloc, zend_ibm_db2_globals, ibm_db2_globals)
	STD_PHP_INI_BOOLEAN("ibm_db2.i5_all_pconnect", "0", PHP_INI_SYSTEM, OnUpdateLong,
		i5_all_pconnect, zend_ibm_db2_globals, ibm_db2_globals)
	STD_PHP_INI_BOOLEAN("ibm_db2.i5_ignore_userid", "0", PHP_INI_SYSTEM, OnUpdateLong,
		i5_ignore_userid, zend_ibm_db2_globals, ibm_db2_globals)
	STD_PHP_INI_BOOLEAN("ibm_db2.i5_job_sort", "0", PHP_INI_SYSTEM, OnUpdateLong,
		i5_job_sort, zend_ibm_db2_globals, ibm_db2_globals)
#endif /* PASE */
	PHP_INI_ENTRY("ibm_db2.instance_name", NULL, PHP_INI_SYSTEM, NULL)
PHP_INI_END()
/* }}} */

#ifdef PASEOUT /* i5 - debug to error log ... _php_db2_errorlog("inlen=%d",tmp_length) */
/* {{{ static void _php_db2_errorlog */
static void _php_db2_errorlog(const char * format, ...)
{
    char bigbuff[4096];
    char * p = (char *) &bigbuff;
    va_list args;
    va_start(args, format);
    vsprintf(p, format, args);
    va_end(args);
    php_error_docref(NULL TSRMLS_CC, E_WARNING, p);
}
/* }}} */
#endif /* PASE */

#ifdef PASE /* i5/OS change ""->NULL */
static void _php_db2_meta_helper(SQLCHAR **qualifier, int *qualifier_len,
				 SQLCHAR **owner,     int *owner_len,
				 SQLCHAR **table,     int *table_len,
				 SQLCHAR **column,    int *column_len)
{
  if (qualifier) {
    	*qualifier=NULL;
    	*qualifier_len=0;
  }
  if (owner) {
    if (**owner=='\0') {
      *owner=NULL;
      *owner_len=0;
    }
    else *owner_len=SQL_NTS;
  }
  if (table) {
    if (**table=='\0') {
      *table=NULL;
      *table_len=0;
    }
    else *table_len=SQL_NTS;
  }
  if (column) {
    if (**column=='\0') {
      *column=NULL;
      *column_len=0;
    }
    else *column_len=SQL_NTS;
  }
}
#endif /* PASE */

/* {{{ static void php_ibm_db2_init_globals
*/
static void php_ibm_db2_init_globals(zend_ibm_db2_globals *ibm_db2_globals)
{
	/* env handle */
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
		rc = SQLFreeHandle(SQL_HANDLE_ENV, handle->henv);
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

			if( prev_ptr->param_type != DB2_PARAM_OUT && prev_ptr->param_type != DB2_PARAM_INOUT ) {
				if (prev_ptr->value) {
					zval_ptr_dtor(&prev_ptr->value);
				}
			}
			efree(prev_ptr);

			prev_ptr = curr_ptr;
		}

		handle->head_cache_list = NULL;

		/* free row data cache */
		if (handle->row_data) {
			for (i=0; i<handle->num_columns;i++) {
				switch (handle->column_info[i].type) {
				case SQL_CHAR:
				case SQL_VARCHAR:
#ifdef PASE
				case SQL_UTF8_CHAR:
#endif
				case SQL_WCHAR:
				case SQL_WVARCHAR:
				case SQL_GRAPHIC:
				case SQL_VARGRAPHIC:
#ifndef PASE /* i5/OS SQL_LONGVARCHAR is SQL_VARCHAR */
				case SQL_LONGVARCHAR:
				case SQL_WLONGVARCHAR:
				case SQL_LONGVARGRAPHIC:
#endif /* PASE */
				case SQL_TYPE_DATE:
				case SQL_TYPE_TIME:
				case SQL_TYPE_TIMESTAMP:
				case SQL_DATETIME:
				case SQL_BIGINT:
				case SQL_DECIMAL:
				case SQL_NUMERIC:
				case SQL_XML:
				case SQL_DECFLOAT:
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

	stmt_res = (stmt_handle *)ecalloc(1, sizeof(stmt_handle));

	/* Initialize stmt resource so parsing assigns updated options if needed */
	stmt_res->hdbc = conn_res->hdbc;
	stmt_res->s_bin_mode = conn_res->c_bin_mode;
	stmt_res->cursor_type = conn_res->c_cursor_type;
	stmt_res->s_case_mode = conn_res->c_case_mode;
#ifdef PASE /* i5 override php.ini */
	stmt_res->s_i5_allow_commit = conn_res->c_i5_allow_commit;
	stmt_res->s_i5_dbcs_alloc   = conn_res->c_i5_dbcs_alloc;
	stmt_res->s_i5_sys_naming   = conn_res->c_i5_sys_naming;
#endif /* PASE */

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
#ifndef PHP_WIN32
	/* Declare variables for DB2 instance settings */
	char * tmp_name = NULL;
	char * instance_name = NULL;
#endif

	ZEND_INIT_MODULE_GLOBALS(ibm_db2, php_ibm_db2_init_globals, NULL);

#ifdef PASE /* i5OS db2_setoptions */
	REGISTER_LONG_CONSTANT("DB2_I5_FETCH_ON", SQL_TRUE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FETCH_OFF", SQL_FALSE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_NAMING_ON",  SQL_TRUE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_NAMING_OFF", SQL_FALSE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_JOB_SORT_ON",  SQL_TRUE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_JOB_SORT_OFF", SQL_FALSE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_DBCS_ALLOC_ON",  SQL_TRUE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_DBCS_ALLOC_OFF", SQL_FALSE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_TXN_NO_COMMIT", SQL_TXN_NO_COMMIT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_TXN_READ_UNCOMMITTED", SQL_TXN_READ_UNCOMMITTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_TXN_READ_COMMITTED", SQL_TXN_READ_COMMITTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_TXN_REPEATABLE_READ", SQL_TXN_REPEATABLE_READ, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_TXN_SERIALIZABLE", SQL_TXN_SERIALIZABLE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_ISO", SQL_FMT_ISO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_USA", SQL_FMT_USA, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_EUR", SQL_FMT_EUR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_JIS", SQL_FMT_JIS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_DMY", SQL_FMT_DMY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_MDY", SQL_FMT_MDY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_YMD", SQL_FMT_YMD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_JUL", SQL_FMT_JUL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_JOB", SQL_FMT_JOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_FMT_HMS", SQL_FMT_HMS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_SLASH", SQL_SEP_SLASH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_DASH", SQL_SEP_DASH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_PERIOD", SQL_SEP_PERIOD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_COMMA", SQL_SEP_COMMA, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_BLANK", SQL_SEP_BLANK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_JOB", SQL_SEP_JOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_I5_SEP_COLON", SQL_SEP_COLON, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_FIRST_IO", SQL_FIRST_IO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_ALL_IO",   SQL_ALL_IO, CONST_CS | CONST_PERSISTENT);
#endif /* PASE */

	REGISTER_LONG_CONSTANT("DB2_BINARY", 1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_CONVERT", 2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PASSTHRU", 3, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_SCROLLABLE", DB2_SCROLLABLE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_FORWARD_ONLY", DB2_FORWARD_ONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PARAM_IN", SQL_PARAM_INPUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PARAM_OUT", SQL_PARAM_OUTPUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_PARAM_INOUT", SQL_PARAM_INPUT_OUTPUT, CONST_CS | CONST_PERSISTENT);
	/* This number chosen is just a place holder to decide binding function to call */
	REGISTER_LONG_CONSTANT("DB2_PARAM_FILE", 11, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_TRUSTED_CONTEXT_ENABLE", SQL_ATTR_USE_TRUSTED_CONTEXT, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DB2_AUTOCOMMIT_ON", SQL_AUTOCOMMIT_ON, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_AUTOCOMMIT_OFF", SQL_AUTOCOMMIT_OFF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_ROWCOUNT_PREFETCH_ON", DB2_ROWCOUNT_PREFETCH_ON, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_ROWCOUNT_PREFETCH_OFF", DB2_ROWCOUNT_PREFETCH_OFF, CONST_CS | CONST_PERSISTENT);
#ifndef PASE
	REGISTER_LONG_CONSTANT("DB2_DEFERRED_PREPARE_ON", SQL_DEFERRED_PREPARE_ON, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_DEFERRED_PREPARE_OFF", SQL_DEFERRED_PREPARE_OFF, CONST_CS | CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("DB2_DOUBLE", SQL_DOUBLE, CONST_CS | CONST_PERSISTENT);
	/* This is how CLI defines SQL_C_LONG */
	REGISTER_LONG_CONSTANT("DB2_LONG", SQL_INTEGER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_CHAR", SQL_CHAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_XML", SQL_XML, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DB2_CASE_NATURAL", 0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_CASE_LOWER", 1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DB2_CASE_UPPER", 2, CONST_CS | CONST_PERSISTENT);

	REGISTER_INI_ENTRIES();

#ifndef PHP_WIN32
	/* Tell DB2 where to find its libraries */
	tmp_name = INI_STR("ibm_db2.instance_name");
	if (NULL != tmp_name) {
		instance_name = (char *)malloc(strlen(DB2_VAR_INSTANCE) + strlen(tmp_name) + 1);
		strcpy(instance_name, DB2_VAR_INSTANCE);
		strcat(instance_name, tmp_name);
		putenv(instance_name);
		_php_db2_instance_name = instance_name;
	}
#endif

#ifdef _AIX
	/* atexit() handler in the DB2/AIX library segfaults in PHP CLI */
	/* DB2NOEXITLIST env variable prevents DB2 from invoking atexit() */
	putenv("DB2NOEXITLIST=TRUE");
#endif

	le_conn_struct = zend_register_list_destructors_ex( _php_db2_free_conn_struct, NULL, DB2_CONN_NAME, module_number);
	le_pconn_struct = zend_register_list_destructors_ex(NULL, _php_db2_free_pconn_struct, DB2_PCONN_NAME, module_number);
	le_stmt_struct = zend_register_list_destructors_ex( _php_db2_free_stmt_struct, NULL, DB2_STMT_NAME, module_number);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(ibm_db2)
{
	UNREGISTER_INI_ENTRIES();

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
	php_info_print_table_header(2, "IBM DB2, Cloudscape and Apache Derby support", "enabled");
	php_info_print_table_row(2, "Module release", PHP_IBM_DB2_VERSION);
	php_info_print_table_row(2, "Module revision", "$Revision$");

	switch (IBM_DB2_G(bin_mode)) {
		case DB2_BINARY:
			php_info_print_table_row(2, "Binary data mode (ibm_db2.binmode)", "DB2_BINARY");
		break;

		case DB2_CONVERT:
			php_info_print_table_row(2, "Binary data mode (ibm_db2.binmode)", "DB2_CONVERT");
		break;

		case DB2_PASSTHRU:
			php_info_print_table_row(2, "Binary data mode (ibm_db2.binmode)", "DB2_PASSTHRU");
		break;
	}
#ifndef PHP_WIN32
	php_info_print_table_row(2, "DB2 instance name (ibm_db2.instance_name)", INI_STR("ibm_db2.instance_name"));
#endif
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
	SQLINTEGER sqlcode = 0;
	SQLSMALLINT length = 0;
	SQLCHAR *p = NULL;

	memset(msg, '\0', SQL_MAX_MESSAGE_LENGTH + 1);
	memset(sqlstate, '\0', SQL_SQLSTATE_SIZE + 1);
	memset(errMsg, '\0', DB2_MAX_ERR_MSG_LEN);

	if ( SQLGetDiagRec(hType, handle, recno, sqlstate, &sqlcode, msg,
			SQL_MAX_MESSAGE_LENGTH + 1, &length)  == SQL_SUCCESS ) {

#ifdef PHP_WIN32
		if (msg[length-2] == '\r') {
			p = &msg[length-2];
			*p = '\0';
		}
#endif
		if (msg[length-1] == '\n') {
			p = &msg[length-1];
			*p = '\0';
		}

		sprintf((char *)errMsg, "%s SQLCODE=%d", msg, (int)sqlcode);

		switch (rc) {
			case SQL_ERROR:
				/* Need to copy the error msg and sqlstate into the symbol Table to cache these results */
				if ( cpy_to_global ) {
					switch (hType) {
						case SQL_HANDLE_DBC:
							strlcpy(IBM_DB2_G(__php_conn_err_state), (char *)sqlstate, SQL_SQLSTATE_SIZE + 1);
							strlcpy(IBM_DB2_G(__php_conn_err_msg), (char *)errMsg, DB2_MAX_ERR_MSG_LEN);
							break;

						case SQL_HANDLE_STMT:
							strlcpy(IBM_DB2_G(__php_stmt_err_state), (char *)sqlstate, SQL_SQLSTATE_SIZE + 1);
							strlcpy(IBM_DB2_G(__php_stmt_err_msg), (char *)errMsg, DB2_MAX_ERR_MSG_LEN);
							break;
					}
				}

				/* This call was made from db2_errmsg or db2_error */
				/* Check for error and return */
				switch (API) {
					case DB2_ERR:
						if ( ret_str != NULL ) {
							strlcpy(ret_str, (char *)sqlstate, SQL_SQLSTATE_SIZE + 1);
						}
						return;
					case DB2_ERRMSG:
						if ( ret_str != NULL ) {
							strlcpy(ret_str, (char *)msg, DB2_MAX_ERR_MSG_LEN);
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


#ifdef PASE /* set the i5/OS library list */
/* {{{ static void _php_db2_i5cmd_helper(void *handle)
*/
static void _php_db2_i5cmd_helper(void *handle)
{
    char *stmt_string;
    int stmt_string_len;
    stmt_handle *stmt_res;
    conn_handle *conn_res=(conn_handle *)handle;
    int rc;
    char len[40];
    char *len_string=(char *)&len;
    int len_string_len;
    char query[32702];
    char *query_string=(char *)&query;
    int query_string_len;

	if (!conn_res || !conn_res->c_i5_pending_cmd) {
		return;
	}

    _php_db2_clear_stmt_err_cache(TSRMLS_C);

    /* setup call to QSYS2/QCMDEXC('cmd',cmdlength) */
    stmt_string = conn_res->c_i5_pending_cmd;
    stmt_string_len = strlen(stmt_string);
    memset(len_string, 0, sizeof(len));
    len_string_len = sprintf(len_string, "%d", stmt_string_len);
    query_string_len = 20 + stmt_string_len + 2 + len_string_len + 1;
    if (query_string_len+1>sizeof(query)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement command exceeds 32702 bytes");
		efree(conn_res->c_i5_pending_cmd);
		conn_res->c_i5_pending_cmd=NULL;
		return;
    }
    memset(query_string,0,sizeof(query));
    switch (conn_res->c_i5_sys_naming) {
		case DB2_I5_NAMING_ON:
			strcpy(query_string,
				"CALL QSYS2/QCMDEXC('"); /* +20 */
			break;
		case DB2_I5_NAMING_OFF:
		default:
			strcpy(query_string,
				"CALL QSYS2.QCMDEXC('"); /* +20 */
			break;
	}
    strcat(query_string, stmt_string);     /* +stmt_string_len */
    strcat(query_string, "',");            /* +2  */
    strcat(query_string, len_string);      /* +len_string_len */
    strcat(query_string, ")");             /* +1  */
					      /* +1 null term */

    stmt_res = _db2_new_stmt_struct(conn_res);

    /* alloc handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
    if ( rc < SQL_SUCCESS ) {
		_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
    }

    rc = SQLExecDirect((SQLHSTMT)stmt_res->hstmt, query_string, query_string_len);
    if ( rc == SQL_ERROR ) {
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
    }
    if ( rc < SQL_SUCCESS ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
    }

    SQLFreeHandle( SQL_HANDLE_STMT, stmt_res->hstmt );
    efree(stmt_res);
    efree(conn_res->c_i5_pending_cmd);
    conn_res->c_i5_pending_cmd = NULL;
}
/* }}} */

#endif /* PASE */

/* {{{ static void _php_db2_assign_options( void *handle, int type, char *opt_key, zval **data )
	*/
static void _php_db2_assign_options( void *handle, int type, char *opt_key, zval **data TSRMLS_DC )
{
	int rc = 0;
	SQLINTEGER pvParam;
	long option_num = 0;
	char *option_str = NULL;

	if (Z_TYPE_PP(data) == IS_STRING) {
		option_str = Z_STRVAL_PP(data);
	} else {
		option_num = Z_LVAL_PP(data);
	}

	if ( !STRCASECMP(opt_key, "cursor")) {
		switch (type) {
			case SQL_HANDLE_DBC:
				switch (option_num) {
					case DB2_SCROLLABLE:
						((conn_handle*)handle)->c_cursor_type = DB2_SCROLLABLE;
						break;

					case DB2_FORWARD_ONLY:
						((conn_handle*)handle)->c_cursor_type = DB2_FORWARD_ONLY;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "CURSOR statement attribute value must be one of DB2_SCROLLABLE or DB2_FORWARD_ONLY");
						break;
				}

			case SQL_HANDLE_STMT:
				if (((stmt_handle *)handle)->cursor_type != option_num ) {
					SQLINTEGER vParam;
					switch (option_num) {
						case DB2_SCROLLABLE:
#ifdef PASE
							vParam = DB2_SCROLLABLE;
							((stmt_handle *)handle)->cursor_type = DB2_SCROLLABLE;
							rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
								SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&vParam,
								SQL_IS_INTEGER );
							if ( rc == SQL_ERROR ) {
								_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
							}
#else
							if (is_ios != 1) {		/* Not i5 */
								vParam = SQL_CURSOR_KEYSET_DRIVEN;
								((stmt_handle *)handle)->cursor_type = DB2_SCROLLABLE;
								rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
									SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)vParam,
									SQL_IS_INTEGER );
								if ( rc == SQL_ERROR ) {
									_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
								}
							} else {				/* Is i5 */
								vParam = DB2_SCROLLABLE;
								((stmt_handle *)handle)->cursor_type = DB2_SCROLLABLE;
								rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
									SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&vParam,
									SQL_IS_INTEGER );
								if ( rc == SQL_ERROR ) {
									_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
								}
							}
#endif
							break;

						case DB2_FORWARD_ONLY:
#ifdef PASE
							vParam = DB2_FORWARD_ONLY;
							rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
								SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&vParam,
								SQL_IS_INTEGER );
							if ( rc == SQL_ERROR ) {
								_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
							}
#else
							if (is_ios != 1) {		/* Not i5 */
								vParam = SQL_SCROLL_FORWARD_ONLY;
								rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
									SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)vParam,
									SQL_IS_INTEGER );
								if ( rc == SQL_ERROR ) {
									_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
								}
							} else {				/* Is i5 */
								vParam = DB2_FORWARD_ONLY;
								rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
									SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&vParam,
									SQL_IS_INTEGER );
								if ( rc == SQL_ERROR ) {
									_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
								}
							}
#endif
							((stmt_handle *)handle)->cursor_type = DB2_FORWARD_ONLY;
							break;

						default:
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "CURSOR statement attribute value must be one of DB2_SCROLLABLE or DB2_FORWARD_ONLY");
							break;
					}
				}
				break;

			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "CURSOR attribute can only be set on connection or statement resources");
		}
	} else 	if ( !STRCASECMP(opt_key, "rowcount")) {
		if (type == SQL_HANDLE_STMT) {
			if (((stmt_handle *)handle)->s_rowcount != option_num ) {
				SQLINTEGER vParam;
				switch (option_num) {
					case DB2_ROWCOUNT_PREFETCH_ON:
#ifdef PASE
						vParam = DB2_ROWCOUNT_PREFETCH_ON;
						((stmt_handle *)handle)->s_rowcount = DB2_ROWCOUNT_PREFETCH_ON;
						rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
							SQL_ATTR_ROWCOUNT_PREFETCH, (SQLPOINTER)&vParam,
							SQL_IS_INTEGER );
						if ( rc == SQL_ERROR ) {
							_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
						}
#else
						if (is_ios != 1) {		/* Not i5 */
							vParam = DB2_ROWCOUNT_PREFETCH_ON;
							((stmt_handle *)handle)->s_rowcount = DB2_ROWCOUNT_PREFETCH_ON;
							rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
								SQL_ATTR_ROWCOUNT_PREFETCH, (SQLPOINTER)vParam,
								SQL_IS_INTEGER );
							if ( rc == SQL_ERROR ) {
								_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
							}
						} else {				/* Is i5 */
							vParam = DB2_ROWCOUNT_PREFETCH_ON;
							((stmt_handle *)handle)->s_rowcount = DB2_ROWCOUNT_PREFETCH_ON;
							rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
								SQL_ATTR_ROWCOUNT_PREFETCH, (SQLPOINTER)&vParam,
								SQL_IS_INTEGER );
							if ( rc == SQL_ERROR ) {
								_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
							}
						}
#endif
						break;

					case DB2_ROWCOUNT_PREFETCH_OFF:
#ifdef PASE
						vParam = DB2_ROWCOUNT_PREFETCH_OFF;
						rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
							SQL_ATTR_ROWCOUNT_PREFETCH, (SQLPOINTER)&vParam,
							SQL_IS_INTEGER );
						if ( rc == SQL_ERROR ) {
							_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
						}
#else
						if (is_ios != 1) {		/* Not i5 */
							vParam = DB2_ROWCOUNT_PREFETCH_OFF;
							rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
								SQL_ATTR_ROWCOUNT_PREFETCH, (SQLPOINTER)vParam,
								SQL_IS_INTEGER );
							if ( rc == SQL_ERROR ) {
								_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
							}
						} else {				/* Is i5 */
							vParam = DB2_ROWCOUNT_PREFETCH_OFF;
							rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
								SQL_ATTR_ROWCOUNT_PREFETCH, (SQLPOINTER)&vParam,
								SQL_IS_INTEGER );
							if ( rc == SQL_ERROR ) {
								_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
							}
						}
#endif
						((stmt_handle *)handle)->s_rowcount = DB2_ROWCOUNT_PREFETCH_OFF;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "ROWCOUNT statement attribute value must be one of DB2_ROWCOUNT_PREFETCH_ON or DB2_ROWCOUNT_PREFETCH_OFF");
						break;
				}
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "ROWCOUNT statement attribute can only be set on statement resources");
		}
	} else if (!STRCASECMP(opt_key, "trustedcontext")) {
		if (type == SQL_HANDLE_DBC ) { /* Checking for connection resource */
			if(((conn_handle*)handle)->handle_active == 0) { /* Checking for live connection */
				switch (option_num) {
					case DB2_TRUSTED_CONTEXT_ENABLE:
						rc = SQLSetConnectAttr(
								(SQLHDBC)((conn_handle*)handle)->hdbc, 
								SQL_ATTR_USE_TRUSTED_CONTEXT, 
								(SQLPOINTER)SQL_TRUE, SQL_IS_INTEGER);

						if ( rc == SQL_ERROR ) {
							_php_db2_check_sql_errors((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
						}
						break;
					
					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED CONTEXT connection attribute value must be DB2_TRUSTED_CONTEXT_ENABLE");
						break;
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED CONTEXT connection attribute can only be set on connection resources");
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED CONTEXT connection attribute can only be set while creating connection");
		}
	} else if (!STRCASECMP(opt_key, "trusted_user")) {
		if (type == SQL_HANDLE_DBC ) { /* Checking for connection resource */
			if(((conn_handle*)handle)->handle_active == 1) { /* Checking for live connection */
				
				int val;
				rc = SQLGetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_USE_TRUSTED_CONTEXT, (SQLPOINTER)&val, 0, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				
				if(val == SQL_TRUE) { /* Checking for Trusted context */
					rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_TRUSTED_CONTEXT_USERID, (SQLPOINTER) option_str, SQL_NTS);
					
					if ( rc == SQL_ERROR ) {
						_php_db2_check_sql_errors((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					}
				} else {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED USER attribute can only be set when TRUSTED CONTEXT is enabled");
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED USER attribute can only be set on live connection");
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED USER attribute can only be set on connection resources");
		}
	} else if (!STRCASECMP(opt_key, "trusted_password")) {
		if (type == SQL_HANDLE_DBC ) { /* Checking for connection resource */
			if(((conn_handle*)handle)->handle_active == 1) { /* Checking for live connection */
				
				int val;
				rc = SQLGetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_USE_TRUSTED_CONTEXT, (SQLPOINTER)&val, 0, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}

				if(val == SQL_TRUE) { /* Checking for Trusted context */
					rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_TRUSTED_CONTEXT_PASSWORD, (SQLPOINTER) option_str, SQL_NTS);
					
					if ( rc == SQL_ERROR ) {
						_php_db2_check_sql_errors((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					}
				} else {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED PASSWORD attribute can only be set when TRUSTED CONTEXT is enabled");
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED PASSWORD attribute can only be set on live connection");
			} 
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "TRUSTED PASSWORD attribute can only be set on connection resources");
		}
	} else if (!STRCASECMP(opt_key, "query_timeout")) {
		if (type == SQL_HANDLE_STMT) {
#ifdef PASE
			rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle*)handle)->hstmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER) &option_num, SQL_IS_UINTEGER );
#else
			rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle*)handle)->hstmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER) option_num, SQL_IS_UINTEGER );
#endif
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle*)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "QUERY TIMEOUT attribute can only be set on statement resources");
		}
	} else if (!STRCASECMP(opt_key, "autocommit")) {
		if (type == SQL_HANDLE_DBC ) {
			if (((conn_handle *)handle)->auto_commit != option_num) {
				switch (option_num) {
					case DB2_AUTOCOMMIT_ON:
						/* Setting AUTOCOMMIT again here. The user could modify
						   this option, close the connection, and reopen it again
						   with this option.
						*/
						((conn_handle*)handle)->auto_commit = 1;
						pvParam = SQL_AUTOCOMMIT_ON;
#ifdef PASE
						rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)&pvParam, SQL_NTS);
#else
						rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)pvParam, SQL_NTS);
#endif
						if ( rc == SQL_ERROR ) {
							_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
						}
						break;

					case DB2_AUTOCOMMIT_OFF:
						((conn_handle*)handle)->auto_commit = 0;
						pvParam = SQL_AUTOCOMMIT_OFF;
#ifdef PASE
						rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)&pvParam, SQL_NTS);
#else
						rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)pvParam, SQL_NTS);
#endif
						if ( rc == SQL_ERROR ) {
							_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
						}
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "AUTOCOMMIT connection attribute value must be one of DB2_AUTOCOMMIT_ON or DB2_AUTOCOMMIT_OFF");
						break;
				}
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "AUTOCOMMIT connection attribute can only be set on connection resources");
		}
	} else if (!STRCASECMP(opt_key, "binmode")) {
		switch (option_num) {
			case DB2_BINARY:
				switch (type) {
					case SQL_HANDLE_DBC:
						((conn_handle*)handle)->c_bin_mode = DB2_BINARY;
						break;

					case SQL_HANDLE_STMT:
						((stmt_handle *)handle)->s_bin_mode = DB2_BINARY;
						break;

					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "BINMODE attribute can only be set on connection or statement resources");
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
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "BINMODE attribute can only be set on connection or statement resources");
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
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "BINMODE attribute can only be set on connection or statement resources");
				}
				break;
		}
	} else if (!STRCASECMP(opt_key, "db2_attr_case")) {
		switch (type) {
			case SQL_HANDLE_DBC:
				switch (option_num) {
					case DB2_CASE_LOWER:
						((conn_handle*)handle)->c_case_mode = DB2_CASE_LOWER;
						break;
					case DB2_CASE_UPPER:
						((conn_handle*)handle)->c_case_mode = DB2_CASE_UPPER;
						break;
					case DB2_CASE_NATURAL:
						((conn_handle*)handle)->c_case_mode = DB2_CASE_NATURAL;
						break;
					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "DB2_ATTR_CASE attribute must be one of DB2_CASE_LOWER, DB2_CASE_UPPER, or DB2_CASE_NATURAL");
				}
				break;
			case SQL_HANDLE_STMT:
				switch (option_num) {
					case DB2_CASE_LOWER:
						((stmt_handle*)handle)->s_case_mode = DB2_CASE_LOWER;
						break;
					case DB2_CASE_UPPER:
						((stmt_handle*)handle)->s_case_mode = DB2_CASE_UPPER;
						break;
					case DB2_CASE_NATURAL:
						((stmt_handle*)handle)->s_case_mode = DB2_CASE_NATURAL;
						break;
					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "DB2_ATTR_CASE attribute must be one of DB2_CASE_LOWER, DB2_CASE_UPPER, or DB2_CASE_NATURAL");
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "DB2_ATTR_CASE attribute can only be set on connection or statement resources");
		}
#ifdef PASE  /* i5/OS new set options */
	} else if (!STRCASECMP(opt_key, "i5_lib")) {
          /* i5_lib - SQL_ATTR_DBC_DEFAULT_LIB
             A character value that indicates the default library that will be used for resolving unqualified file references. This is not valid if the connection is using system naming mode.
             */
	    char * lib = (char *)option_str;
	    rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_DBC_DEFAULT_LIB, (SQLPOINTER)lib, SQL_NTS);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (!STRCASECMP(opt_key, "i5_libl")) {
		/* i5_libl - call qsys2.qcmdexc('i5command',lengthofi5command)
		CHGLIBL LIBL(IWIKI DB2) CURLIB(IWIKI)
		Set the library list in the DB2 server process.
		*/
	    char i5curlibbuf[1024];
	    char * curlib = (char *)&i5curlibbuf;
	    char * libl = (char *)option_str;
		int libl_len = strlen(libl);

		if (libl_len) {
			int curlib_len = libl_len;
			char * curr_lib_end = strchr(libl, ' ');
			if (curr_lib_end) curlib_len = curr_lib_end-libl;
			libl_len += (13 + libl_len + 9 + curlib_len + 2);

			if (libl_len<32702) {
				char *i5cmd = ((conn_handle*)handle)->c_i5_pending_cmd = (char *) ecalloc(1, libl_len);
				memset(i5cmd, 0, libl_len);
				memset(curlib, 0, sizeof(i5curlibbuf));
				strncpy(curlib, libl, curlib_len);      
				strcpy(i5cmd, "CHGLIBL LIBL("); /* +13             */
				strcat(i5cmd, libl);            /* +strlen(libl)   */
				strcat(i5cmd, ") CURLIB(");     /* +9              */
				strcat(i5cmd, curlib);          /* +strlen(curlib) */
				strcat(i5cmd, ")");             /* +1              */
				/* +1 null term    */
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_libl too long");
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_libl missing library list");
	    }
	} else if (!STRCASECMP(opt_key, "i5_naming")) {
		/* i5_naming - SQL_ATTR_DBC_SYS_NAMING
		DB2_I5_NAMING_ON value turns on DB2 UDB CLI iSeries system naming mode. Files are qualified using the slash (/) delimiter. Unqualified files are resolved using the library list for the job..
		DB2_I5_NAMING_OFF value turns off DB2 UDB CLI default naming mode, which is SQL naming. Files are qualified using the period (.) delimiter. Unqualified files are resolved using either the default library or the current user ID.
		*/
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_NAMING_ON:
			case DB2_I5_NAMING_OFF:
				((conn_handle*)handle)->c_i5_sys_naming = option_num;
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_DBC_SYS_NAMING, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_naming (DB2_I5_NAMING_ON, DB2_I5_NAMING_OFF)");
		}
	} else if (!STRCASECMP(opt_key, "i5_job_sort")) {
		/* i5_job_sort - SQL_ATTR_JOB_SORT_SEQUENCE (conn is hidden 10046)
		DB2_I5_JOB_SORT_ON value turns on DB2 UDB CLI job sort mode. 
		DB2_I5_JOB_SORT_OFF value turns off DB2 UDB CLI job sortmode. 
		*/
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_JOB_SORT_ON:
				pvParam = 2; /* special new value via John Broich PTF */
			case DB2_I5_JOB_SORT_OFF:
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, 10046, (SQLPOINTER)&pvParam, 0);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_job_sort (DB2_I5_JOB_SORT_ON, DB2_I5_JOB_SORT_OFF)");
		}
	} else if (!STRCASECMP(opt_key, "i5_dbcs_alloc")) {
		/* i5_dbcs_alloc SQL_ATTR_DBC_SYS_NAMING
		DB2_I5_DBCS_ALLOC_ON value turns on DB2 6X allocation scheme for DBCS translation column size growth.
		DB2_I5_DBCS_ALLOC_OFF value turns off DB2 6X allocation scheme for DBCS translation column size growth.
		*/
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_DBCS_ALLOC_ON:
				/* override i5_dbcs_alloc in php.ini */
				((conn_handle*)handle)->c_i5_dbcs_alloc = 1;
				break;
			case DB2_I5_DBCS_ALLOC_OFF:
				/* override i5_dbcs_alloc in php.ini */
				((conn_handle*)handle)->c_i5_dbcs_alloc = 0;
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_dbcs_alloc (DB2_I5_DBCS_ALLOC_ON, DB2_I5_DBCS_ALLOC_OFF)");
	    }
	} else if (!STRCASECMP(opt_key, "i5_commit")) {
          /* i5_commit - SQL_ATTR_COMMIT
             The SQL_ATTR_COMMIT attribute should be set before the SQLConnect(). If the value is changed after the connection has been established, and the connection is to a remote data source, the change does not take effect until the next successful SQLConnect() for the connection handle
             DB2_I5_TXN_NO_COMMIT - Commitment control is not used.
             DB2_I5_TXN_READ_UNCOMMITTED - Dirty reads, nonrepeatable reads, and phantoms are possible. 
             DB2_I5_TXN_READ_COMMITTED - Dirty reads are not possible. Nonrepeatable reads, and phantoms are possible. 
             DB2_I5_TXN_REPEATABLE_READ - Dirty reads and nonrepeatable reads are not possible. Phantoms are possible. 
             DB2_I5_TXN_SERIALIZABLE - Transactions are serializable. Dirty reads, non-repeatable reads, and phantoms are not possible
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_TXN_READ_UNCOMMITTED:
			case DB2_I5_TXN_READ_COMMITTED:
			case DB2_I5_TXN_REPEATABLE_READ:
			case DB2_I5_TXN_SERIALIZABLE:
				/* override commit in php.ini */
				((conn_handle*)handle)->c_i5_allow_commit = 1;
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_COMMIT, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			case DB2_I5_TXN_NO_COMMIT:
				/* override commit in php.ini */
				((conn_handle*)handle)->c_i5_allow_commit = 0;
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_COMMIT, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_commit (DB2_I5_TXN_NO_COMMIT, DB2_I5_TXN_READ_UNCOMMITTED, DB2_I5_TXN_READ_COMMITTED, DB2_I5_TXN_REPEATABLE_READ, DB2_I5_TXN_SERIALIZABLE)");
	    }

	} else if (!STRCASECMP(opt_key, "i5_date_fmt")) {
          /* i5_date_fmt - SQL_ATTR_DATE_FMT
             DB2_I5_FMT_ISO - The International Organization for Standardization (ISO) date format yyyy-mm-dd is used. This is the default. 
             DB2_I5_FMT_USA - The United States date format mm/dd/yyyy is used. 
             DB2_I5_FMT_EUR - The European date format dd.mm.yyyy is used. 
             DB2_I5_FMT_JIS - The Japanese Industrial Standard date format yyyy-mm-dd is used. 
             DB2_I5_FMT_MDY - The date format mm/dd/yyyy is used. 
             DB2_I5_FMT_DMY - The date format dd/mm/yyyy is used. 
             DB2_I5_FMT_YMD - The date format yy/mm/dd is used. 
             DB2_I5_FMT_JUL - The Julian date format yy/ddd is used. 
             DB2_I5_FMT_JOB - The job default is used.
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_FMT_ISO:
			case DB2_I5_FMT_USA:
			case DB2_I5_FMT_EUR:
			case DB2_I5_FMT_JIS:
			case DB2_I5_FMT_MDY:
			case DB2_I5_FMT_DMY:
			case DB2_I5_FMT_YMD:
			case DB2_I5_FMT_JUL:
			case DB2_I5_FMT_JOB:
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_DATE_FMT, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_date_fmt DB2_I5_FMT_ISO, DB2_I5_FMT_USA, DB2_I5_FMT_EUR, DB2_I5_FMT_JIS, DB2_I5_FMT_MDY, DB2_I5_FMT_DMY, DB2_I5_FMT_YMD, DB2_I5_FMT_JUL, DB2_I5_FMT_JOB");
		}
	} else if (!STRCASECMP(opt_key, "i5_date_sep")) {
          /* i5_date_sep - SQL_ATTR_DATE_SEP
             DB2_I5_SEP_SLASH - A slash ( / ) is used as the date separator. This is the default. 
             DB2_I5_SEP_DASH - A dash ( - ) is used as the date separator. 
             DB2_I5_SEP_PERIOD - A period ( . ) is used as the date separator. 
             DB2_I5_SEP_COMMA - A comma ( , ) is used as the date separator. 
             DB2_I5_SEP_BLANK - A blank is used as the date separator. 
             DB2_I5_SEP_JOB - The job default is used
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_SEP_SLASH:
			case DB2_I5_SEP_DASH:
			case DB2_I5_SEP_PERIOD:
			case DB2_I5_SEP_COMMA:
			case DB2_I5_SEP_BLANK:
			case DB2_I5_SEP_JOB:
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_DATE_SEP, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_date_sep (DB2_I5_SEP_SLASH, DB2_I5_SEP_DASH, DB2_I5_SEP_PERIOD, DB2_I5_SEP_COMMA, DB2_I5_SEP_BLANK, DB2_I5_SEP_JOB)");
	    }
	} else if (!STRCASECMP(opt_key, "i5_time_fmt")) {
          /* i5_time_fmt - SQL_ATTR_TIME_FMT
             DB2_I5_FMT_ISO - The International Organization for Standardization (ISO) time format hh.mm.ss is used. This is the default. 
             DB2_I5_FMT_USA - The United States time format hh:mmxx is used, where xx is AM or PM. 
             DB2_I5_FMT_EUR - The European time format hh.mm.ss is used. 
             DB2_I5_FMT_JIS - The Japanese Industrial Standard time format hh:mm:ss is used. 
             DB2_I5_FMT_HMS - The hh:mm:ss format is used.
             */ 
	    pvParam = option_num;
	    switch (option_num) {	
			case DB2_I5_FMT_ISO:
			case DB2_I5_FMT_USA:
			case DB2_I5_FMT_EUR:
			case DB2_I5_FMT_JIS:
			case DB2_I5_FMT_HMS:
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_TIME_FMT, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_time_fmt (DB2_I5_FMT_ISO, DB2_I5_FMT_USA, DB2_I5_FMT_EUR, DB2_I5_FMT_JIS, DB2_I5_FMT_HMS)");
	    }
	} else if (!STRCASECMP(opt_key, "i5_time_sep")) {
          /* i5_time_sep - SQL_ATTR_TIME_SEP
             DB2_I5_SEP_COLON - A colon ( : ) is used as the time separator. This is the default. 
             DB2_I5_SEP_PERIOD - A period ( . ) is used as the time separator. 
             DB2_I5_SEP_COMMA - A comma ( , ) is used as the time separator. 
             DB2_I5_SEP_BLANK - A blank is used as the time separator. 
             DB2_I5_SEP_JOB - The job default is used.
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_SEP_COLON:
			case DB2_I5_SEP_PERIOD:
			case DB2_I5_SEP_COMMA:
			case DB2_I5_SEP_BLANK:
			case DB2_I5_SEP_JOB:
			    rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_TIME_SEP, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
			    php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_time_sep (DB2_I5_SEP_COLON, DB2_I5_SEP_PERIOD, DB2_I5_SEP_COMMA, DB2_I5_SEP_BLANK, DB2_I5_SEP_JOB)");
		}
	} else if (!STRCASECMP(opt_key, "i5_decimal_sep")) {
          /* i5_decimal_sep - SQL_ATTR_DECIMAL_SEP
             DB2_I5_SEP_PERIOD - A period ( . ) is used as the decimal separator. This is the default. 
             DB2_I5_SEP_COMMA - A comma ( , ) is used as the date separator. 
             DB2_I5_SEP_JOB - The job default is used.
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_SEP_PERIOD:
			case DB2_I5_SEP_COMMA:
			case DB2_I5_SEP_JOB:
			    rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_DECIMAL_SEP, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
			    break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_decimal_sep (DB2_I5_SEP_PERIOD, DB2_I5_SEP_COMMA, DB2_I5_SEP_JOB)");
	    }
	} else if (!STRCASECMP(opt_key, "i5_query_optimize")) {
          /* i5_query_optimize - SQL_ATTR_QUERY_OPTIMIZE_GOAL
             DB2_FIRST_IO All queries are optimized with the goal of returning the first page of output as fast as possible. This goal works well when the output is controlled by a user who is most likely to cancel the query after viewing the first page of output data. Queries coded with an OPTIMIZE FOR nnn ROWS clause honor the goal specified by the clause.
             DB2_ALL_IO All queries are optimized with the goal of running the entire query to completion in the shortest amount of elapsed time. This is a good option when the output of a query is being written to a file or report, or the interface is queuing the output data. Queries coded with an OPTIMIZE FOR nnn ROWS clause honor the goal specified by the clause. This is the default.
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_FIRST_IO:
			case DB2_ALL_IO:
				rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_QUERY_OPTIMIZE_GOAL, (SQLPOINTER)&pvParam, SQL_NTS);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_fetch_only (DB2_FIRST_IO, DB2_ALL_IO)");
	    }
	} else if (!STRCASECMP(opt_key, "i5_fetch_only")) {
          /* i5_fetch_only - SQL_ATTR_FOR_FETCH_ONLY
             DB2_I5_FETCH_ON - Cursors are read-only and cannot be used for positioned updates or deletes. This is the default unless SQL_ATTR_FOR_FETCH_ONLY environment has been set to SQL_FALSE. 
             DB2_I5_FETCH_OFF - Cursors can be used for positioned updates and deletes.
             */
	    pvParam = option_num;
	    switch (option_num) {
			case DB2_I5_FETCH_ON:
			case DB2_I5_FETCH_OFF:
				rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
					SQL_ATTR_FOR_FETCH_ONLY, (SQLPOINTER)&pvParam,
					SQL_IS_INTEGER );
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "i5_fetch_only (DB2_I5_FETCH_ON, DB2_I5_FETCH_OFF)");
		}
#endif /* PASE */
#ifndef PASE  /* i5/OS not support yet */
	} else if (!STRCASECMP(opt_key, "userid")) {
		rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_INFO_USERID, (SQLPOINTER)option_str, SQL_NTS);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (!STRCASECMP(opt_key, "acctstr")) {
		rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_INFO_ACCTSTR, (SQLPOINTER)option_str, SQL_NTS);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (!STRCASECMP(opt_key, "applname")) {
		rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_INFO_APPLNAME, (SQLPOINTER)option_str, SQL_NTS);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (!STRCASECMP(opt_key, "wrkstnname")) {
		rc = SQLSetConnectAttr((SQLHDBC)((conn_handle*)handle)->hdbc, SQL_ATTR_INFO_WRKSTNNAME, (SQLPOINTER)option_str, SQL_NTS);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)((conn_handle*)handle)->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (!STRCASECMP(opt_key, "deferred_prepare")) {
		switch (option_num) {
			case DB2_DEFERRED_PREPARE_ON:
				pvParam = SQL_DEFERRED_PREPARE_ON;
				rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
						SQL_ATTR_DEFERRED_PREPARE, (SQLPOINTER)pvParam,
						SQL_IS_INTEGER );
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case DB2_DEFERRED_PREPARE_OFF:
				pvParam = SQL_DEFERRED_PREPARE_OFF;
				rc = SQLSetStmtAttr((SQLHSTMT)((stmt_handle *)handle)->hstmt,
						SQL_ATTR_DEFERRED_PREPARE, (SQLPOINTER)pvParam,
						SQL_IS_INTEGER );
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)((stmt_handle *)handle)->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "DEFERRED PREPARE statement attribute value must be one of DB2_DEFERRED_PREPARE_ON or DB2_DEFERRED_PREPARE_OFF");
				break;
		}
#endif /* not PASE */
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
	zval **tc_pass = NULL;

	if ( options != NULL) {
		numOpts = zend_hash_num_elements(Z_ARRVAL_P(options));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));

		for ( i = 0; i < numOpts; i++) {
			if (zend_hash_get_current_key(Z_ARRVAL_P(options), &opt_key,
				&num_idx, 1) == HASH_KEY_IS_STRING) {
					zend_hash_get_current_data(Z_ARRVAL_P(options), (void**)&data);
					
					if (!STRCASECMP(opt_key, "trusted_password")) {
						tc_pass = data;
					} else {
						/* Assign options to handle. */
						/* Sets the options in the handle with CLI/ODBC calls */
						_php_db2_assign_options( handle, type, opt_key, data TSRMLS_CC );
					}
					
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
		if(tc_pass != NULL) {
			/* Assign options to handle. */
			/* Sets the options in the handle with CLI/ODBC calls */
			_php_db2_assign_options( handle, type, "trusted_password", tc_pass TSRMLS_CC );
		}
	}
	return 0;
}
/* }}} */

/* {{{ static int _php_db2_get_result_set_info(stmt_handle *stmt_res TSRMLS_DC)
initialize the result set information of each column. This must be done once
*/
static int _php_db2_get_result_set_info(stmt_handle *stmt_res TSRMLS_DC)
{
	int rc = -1, i;
	SQLSMALLINT nResultCols = 0, name_length;
	char tmp_name[BUFSIZ];
	rc = SQLNumResultCols((SQLHSTMT)stmt_res->hstmt, &nResultCols);
	if ( rc == SQL_ERROR || nResultCols == 0) {
		_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		return -1;
	}
	stmt_res->num_columns = nResultCols;
	stmt_res->column_info = (db2_result_set_info*)ecalloc(nResultCols, sizeof(db2_result_set_info));

	/* return a set of attributes for a column */
	for (i = 0 ; i < nResultCols; i++) {
		stmt_res->column_info[i].lob_loc = 0;
		stmt_res->column_info[i].loc_ind = 0;
		stmt_res->column_info[i].loc_type = 0;
		rc = SQLDescribeCol((SQLHSTMT)stmt_res->hstmt, (SQLSMALLINT)(i + 1 ),
			(SQLCHAR *)&tmp_name, BUFSIZ, &name_length, &stmt_res->column_info[i].type,
			&stmt_res->column_info[i].size, &stmt_res->column_info[i].scale,
			&stmt_res->column_info[i].nullable);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			return -1;
		}

		if ( name_length <= 0 ) {
			stmt_res->column_info[i].name = (SQLCHAR *)estrdup("");
		} else if (name_length >= BUFSIZ ) {
			/* column name is longer than BUFSIZ*/
			stmt_res->column_info[i].name = ecalloc(1, name_length+1);
			rc = SQLDescribeCol((SQLHSTMT)stmt_res->hstmt, (SQLSMALLINT)(i + 1 ),
				stmt_res->column_info[i].name, name_length, &name_length,
				&stmt_res->column_info[i].type, &stmt_res->column_info[i].size,
				&stmt_res->column_info[i].scale, &stmt_res->column_info[i].nullable);
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				return -1;
			}
		} else {
			stmt_res->column_info[i].name = (SQLCHAR *)estrdup(tmp_name);
		}
#ifdef PASE /* i5/OS size changes for "common" converts */
		switch (stmt_res->column_info[i].type) {
		        /* BIGINT 9223372036854775807  (2^63-1) string convert */
			case SQL_BIGINT: 
			    stmt_res->column_info[i].size = 20;
			    break;
			default:
			    break;
		}
#endif /* PASE */
#ifdef PASE /* i5/OS DBCS may have up to 6 times growth in column alloc size on convert */
		if (stmt_res->s_i5_dbcs_alloc) {
		    switch (stmt_res->column_info[i].type) {
				case SQL_CHAR:
				case SQL_VARCHAR:
				case SQL_CLOB:
				case SQL_DBCLOB:
				case SQL_UTF8_CHAR:
				case SQL_WCHAR:
				case SQL_WVARCHAR:
				case SQL_GRAPHIC:
				case SQL_VARGRAPHIC:
					stmt_res->column_info[i].size = stmt_res->column_info[i].size * 6;
				default:
					break;
			}
		}
#endif /* PASE */
	}
	return 0;
}
/* }}} */

/* {{{ static int _php_db2_bind_column_helper(stmt_handle *stmt_res)
	bind columns to data, this must be done once
*/
static int _php_db2_bind_column_helper(stmt_handle *stmt_res TSRMLS_DC)
{
	SQLINTEGER in_length = 0;
	SQLSMALLINT column_type;
	db2_row_data_type *row_data;
	int i, rc = SQL_SUCCESS;
	SQLSMALLINT target_type = SQL_C_DEFAULT;

	stmt_res->row_data = (db2_row_type*) ecalloc(stmt_res->num_columns, sizeof(db2_row_type));

	for (i=0; i<stmt_res->num_columns; i++) {
		column_type = stmt_res->column_info[i].type;
		row_data = &stmt_res->row_data[i].data;
		switch(column_type) {
			case SQL_CHAR:
			case SQL_VARCHAR:
#ifdef PASE
			case SQL_UTF8_CHAR:
#endif
			case SQL_WCHAR:
			case SQL_WVARCHAR:
#ifndef PASE /* i5/OS SQL_DBCLOB */
			case SQL_DBCLOB:
#endif /* not PASE */
#ifndef PASE /* i5/OS SQL_LONGVARCHAR is SQL_VARCHAR */
			case SQL_LONGVARCHAR:
			case SQL_WLONGVARCHAR:
			case SQL_LONGVARGRAPHIC:
#endif /* PASE */
				target_type = SQL_C_CHAR;
				in_length = stmt_res->column_info[i].size+1;
				row_data->str_val = (SQLCHAR *)ecalloc(1, in_length);

				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					target_type, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			case SQL_GRAPHIC:
			case SQL_VARGRAPHIC:
				target_type = SQL_C_CHAR;
				in_length = (stmt_res->column_info[i].size + 1) * 2 + 1;
				row_data->str_val = (SQLCHAR *) ecalloc (1, in_length);

				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT) (i + 1),
					target_type, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

#ifdef PASE /* i5/OS v6r1 incompatible change */
			case SQL_VARBINARY_V6:
			case SQL_BINARY_V6:
#endif /* PASE */
			case SQL_BINARY:
#ifndef PASE /* i5/OS SQL_LONGVARBINARY is SQL_VARBINARY */
			case SQL_LONGVARBINARY:
#endif /* PASE */
			case SQL_VARBINARY:
#ifndef PASE /* i5/OS BINARY SQL_C_CHAR errors */
				if ( stmt_res->s_bin_mode == DB2_CONVERT ) {
					in_length = 2 * (stmt_res->column_info[i].size) + 1;
					row_data->str_val = (SQLCHAR *)ecalloc(1, in_length);

					rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
						SQL_C_CHAR, row_data->str_val, in_length,
						(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
					if ( rc == SQL_ERROR ) {
						_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
					}
				} else {
#endif /* PASE */
					in_length = stmt_res->column_info[i].size + 1;
					row_data->str_val = (SQLCHAR *)ecalloc(1, in_length);

#ifdef PASE /* i5/OS VARBINARY to SQL_C_BINARY also work for BINARY */
					rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
						SQL_C_BINARY, row_data->str_val, in_length,
						(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
#else					
					rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
						SQL_C_DEFAULT, row_data->str_val, in_length,
						(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
#endif
					if ( rc == SQL_ERROR ) {
						_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
					}
#ifndef PASE /* i5/OS BINARY SQL_C_CHAR errors */
				}
#endif /* PASE */
				break;
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
			case SQL_TYPE_TIMESTAMP:
			case SQL_DATETIME:
			case SQL_BIGINT:
			case SQL_DECFLOAT:
				in_length = stmt_res->column_info[i].size + 2;
				if(column_type == SQL_BIGINT) {
					in_length++;
				}
				row_data->str_val = (SQLCHAR *)ecalloc(1, in_length);
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_CHAR, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case SQL_SMALLINT:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_DEFAULT, &row_data->s_val, sizeof(row_data->s_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case SQL_INTEGER:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_DEFAULT, &row_data->i_val, sizeof(row_data->i_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case SQL_REAL:
#ifndef PASE /* need this LUW? */
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_FLOAT, &row_data->r_val, sizeof(row_data->r_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
#endif
			case SQL_FLOAT:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_DEFAULT, &row_data->f_val, sizeof(row_data->f_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case SQL_DOUBLE:
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_DEFAULT, &row_data->d_val, sizeof(row_data->d_val),
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case SQL_DECIMAL:
			case SQL_NUMERIC:
				in_length = stmt_res->column_info[i].size +
					stmt_res->column_info[i].scale + 2 + 1;
				row_data->str_val = (SQLCHAR *)ecalloc(1, in_length);
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
					SQL_C_CHAR, row_data->str_val, in_length,
					(SQLINTEGER *)(&stmt_res->row_data[i].out_length));
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;

			case SQL_CLOB:
				stmt_res->row_data[i].out_length = 0;
				stmt_res->column_info[i].loc_type = SQL_CLOB_LOCATOR;
				stmt_res->column_info[i].loc_ind = 0;
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
						stmt_res->column_info[i].loc_type, &stmt_res->column_info[i].lob_loc, 4,
						&stmt_res->column_info[i].loc_ind);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
				break;
			case SQL_BLOB:
				stmt_res->row_data[i].out_length = 0;
				stmt_res->column_info[i].loc_type = SQL_BLOB_LOCATOR;
				stmt_res->column_info[i].loc_ind = 0;
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
						stmt_res->column_info[i].loc_type, &stmt_res->column_info[i].lob_loc, 4,
						&stmt_res->column_info[i].loc_ind);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
 			   break;
#ifdef PASE /* i5/OS SQL_DBCLOB */
			case SQL_DBCLOB:
				stmt_res->row_data[i].out_length = 0;
				stmt_res->column_info[i].loc_type = SQL_DBCLOB_LOCATOR;
				stmt_res->column_info[i].loc_ind = 0;
				rc = SQLBindCol((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)(i + 1),
						stmt_res->column_info[i].loc_type, &stmt_res->column_info[i].lob_loc, 4,
						&stmt_res->column_info[i].loc_ind);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
 			   break;
#endif /* PASE */
			case SQL_XML:
				stmt_res->row_data[i].out_length = 0;
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
	int database_len;
	int uid_len;
	int password_len;
	zval *options = NULL;
	int rc = 0;
	SQLINTEGER conn_alive;
	conn_handle *conn_res = *pconn_res;
	int reused = 0;
	int hKeyLen = 0;
	char *hKey = NULL;
	list_entry newEntry;
	char server[2048];
#ifdef PASE /* i5/OS incompatible v6 change */
	long attr = SQL_TRUE;
	char buffer11[11];
	int conn_null = 0;
	int conn_pclose=0;
#endif /* PASE */

	conn_alive = 1;

	if (zend_parse_parameters(argc TSRMLS_CC, "sss|a", &database, &database_len,&uid,
		&uid_len, &password, &password_len, &options) == FAILURE) {
		return -1;
	}

#ifdef PASE 
	/* 1) i5/OS DB2 security PTF change (compatibility workaround):
	 *    a) (p)connect(...,"","") promotes (p)connect(...,null,null)
	 *    b) (p)connect(...,"ANYUID","") is no longer supported.
	 * 2) random connect failures
	 *    mixing db2_(p)connect(...,null,null) in-process mode 
	 *    and db2_(p)connect(...,"uid","pwd") server mode is
	 *    not allowed. However, php.ini ibm_db2.i5_ignore_userid 
	 *    promotes all (p)connects to (p)connect(...,null,null).
	 * 3) random connect / prepared statement failures
	 *    switching DB2_I5_NAMING_ON/OFF in a persistent
	 *    connection produces bad results. No action
	 *    is planned at this time (user error) ...
	 *    adding to hKey would allow for 2 connections.
	 */
	if (IBM_DB2_G(i5_ignore_userid) || (!uid_len && !password_len)) {
		conn_null = 1;
	}
#endif /* PASE */
	do {
		/* Check if we already have a connection for this userID & database combination */
		if (isPersistent) {
			list_entry *entry;
			hKeyLen = strlen(database) + strlen(uid) + strlen(password) + 9;
			hKey = (char *) ecalloc(1, hKeyLen);

			sprintf(hKey, "__db2_%s.%s.%s", uid, database, password);

			if (zend_hash_find(&EG(persistent_list), hKey, hKeyLen, (void **) &entry) == SUCCESS) {
				conn_res = *pconn_res = (conn_handle *) entry->ptr;
#ifndef PASE /* i5/OS not issue local */
				/* Need to reinitialize connection? */
				rc = SQLGetConnectAttr(conn_res->hdbc, SQL_ATTR_PING_DB, (SQLPOINTER)&conn_alive, 0, NULL);
				if ( (rc == SQL_SUCCESS) && conn_alive ) {
					_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					reused = 1;
				} /* else will re-connect since connection is dead */
#endif /* PASE */
				reused = 1;
			}
		} else {
			/* Need to check for max pconnections? */
		}
		if (*pconn_res == NULL) {
			conn_res = *pconn_res =
				(conn_handle *) (isPersistent ?  pecalloc(1, sizeof(conn_handle), 1) : ecalloc(1, sizeof(conn_handle)));
		}
		/* We need to set this early, in case we get an error below,
			so we know how to free the connection */
#ifdef PASE /* i5/OS pclose */
		if (conn_res->flag_pconnect == 9) {
			reused = 0;
			conn_pclose = 1;
		}
#endif /* PASE */
		conn_res->flag_pconnect = isPersistent;
		/* Allocate ENV handles if not present */
		if ( !conn_res->henv ) {
			rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &(conn_res->henv));
			if (rc != SQL_SUCCESS) {
				_php_db2_check_sql_errors( conn_res->henv, SQL_HANDLE_ENV, rc, 1, NULL, -1, 1 TSRMLS_CC);
				break;
			}
#ifndef PASE /* i5/OS difference */
			rc = SQLSetEnvAttr((SQLHENV)conn_res->henv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
#else
			/* enter server mode (connect QSQSRVR jobs) */
			if (!(IBM_DB2_G(i5_ignore_userid)) && strstr(database, "=") == NULL) {
				if (!is_i5_server_mode) {
					attr = SQL_TRUE;
					rc = SQLSetEnvAttr((SQLHENV)conn_res->henv, SQL_ATTR_SERVER_MODE, &attr, 0);
					if (rc == SQL_SUCCESS) {
						is_i5_server_mode=1;
					}
				}
			}
#endif /* PASE */
		}

		if (! reused) {
			/* Alloc CONNECT Handle */
			rc = SQLAllocHandle( SQL_HANDLE_DBC, conn_res->henv, &(conn_res->hdbc));
			if (rc != SQL_SUCCESS) {
				_php_db2_check_sql_errors(conn_res->henv, SQL_HANDLE_ENV, rc, 1, NULL, -1, 1 TSRMLS_CC);
				break;
			}
		}

		/* Set this after the connection handle has been allocated to avoid
		unnecessary network flows. Initialize the structure to default values */
		conn_res->auto_commit = SQL_AUTOCOMMIT_ON;
#ifndef PASE
		rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)(conn_res->auto_commit), SQL_NTS);
#else /* PASE */
		rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)(&conn_res->auto_commit), SQL_NTS);
		conn_res->c_i5_allow_commit = IBM_DB2_G(i5_allow_commit);
		conn_res->c_i5_dbcs_alloc = IBM_DB2_G(i5_dbcs_alloc);
		if (IBM_DB2_G(i5_job_sort)) {
			attr = 2; /* 2 = special value John Broich PTF */
			rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, 10046, (SQLPOINTER)&attr, 0);
		}
#endif /* PASE */

		conn_res->c_bin_mode = IBM_DB2_G(bin_mode);
		conn_res->c_case_mode = DB2_CASE_NATURAL;
		conn_res->c_cursor_type = DB2_FORWARD_ONLY;

		conn_res->error_recno_tracker = 1;
		conn_res->errormsg_recno_tracker = 1;

		/* handle not active as of yet */
		conn_res->handle_active = 0;
#ifdef PASE
		conn_res->c_i5_pending_cmd=NULL;
#endif /* PASE */
		/* Set Options */
		if ( options != NULL ) {
			rc = _php_db2_parse_options( options, SQL_HANDLE_DBC, conn_res TSRMLS_CC );
			if (rc != SQL_SUCCESS) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Options Array must have string indexes");
			}
		}
#ifdef PASE
		if (!conn_res->c_i5_allow_commit) {
			if (!rc) {
				SQLINTEGER nocommitpase = SQL_TXN_NO_COMMIT;
				SQLSetConnectOption((SQLHDBC)conn_res->hdbc, SQL_ATTR_COMMIT, (SQLPOINTER)&nocommitpase);
			}
		}
#endif /* PASE */

		if (! reused) {
			/* Connect */
			/* If the string contains a =, use SQLDriverConnect */
			if ( strstr(database, "=") != NULL ) {
				rc = SQLDriverConnect((SQLHDBC)conn_res->hdbc, (SQLHWND)NULL,
						(SQLCHAR*)database, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );
			} else {
#ifdef PASE /* i5/OS security PTF compatabilty workaround */
				if (conn_null) {
					rc = SQLConnect( (SQLHDBC)conn_res->hdbc, (SQLCHAR *)database,
						(SQLSMALLINT)database_len, (SQLCHAR *)NULL, (SQLSMALLINT)0,
						(SQLCHAR *)NULL, (SQLSMALLINT)0);

				} 
				else 
#endif /* PASE */
				rc = SQLConnect( (SQLHDBC)conn_res->hdbc, (SQLCHAR *)database,
						(SQLSMALLINT)database_len, (SQLCHAR *)uid, (SQLSMALLINT)uid_len,
						(SQLCHAR *)password, (SQLSMALLINT)password_len );
			}

			if ( rc != SQL_SUCCESS ) {
				_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				SQLFreeHandle( SQL_HANDLE_DBC, conn_res->hdbc );
				SQLFreeHandle(SQL_HANDLE_ENV, conn_res->henv);
				break;
			}
#ifdef CLI_DBC_SERVER_TYPE_DB2LUW
#ifdef SQL_ATTR_DECFLOAT_ROUNDING_MODE
			/**
			 * Code for setting SQL_ATTR_DECFLOAT_ROUNDING_MODE
			 * for implementation of Decfloat Datatype
			 * */
			_php_db2_set_decfloat_rounding_mode_client(conn_res TSRMLS_CC);
#endif
#endif
			/* Get the server name */
			memset(server, 0, sizeof(server));
			rc = SQLGetInfo(conn_res->hdbc, SQL_DBMS_NAME, (SQLPOINTER)server, 2048, NULL);
			if (!strcmp(server, "AS")) {
				is_ios = 1;
			}
			if (!strcmp(server, "DB2")) {
				is_zos = 1;
			}

#ifdef PASE /* i5/OS incompatible v6+ change */
			memset(buffer11, 0, sizeof(buffer11));
			rc = SQLGetInfo(conn_res->hdbc, SQL_DBMS_VER, (SQLPOINTER)buffer11, sizeof(buffer11), NULL);
			if (buffer11[0]=='0' && buffer11[1]=='5') {
				is_i5os_classic = 1;
			} else {
				is_i5os_classic = 0;
			}
			/* reused pclose, so zend_hash still good */
			if (conn_pclose) {
				reused = 1;
			}
#endif /* PASE */
		}
		conn_res->handle_active = 1;
	} while (0);

	if (hKey != NULL) {
		if (! reused && rc == SQL_SUCCESS) {
			/* If we created a new persistent connection, add it to the persistent_list */
			memset(&newEntry, 0, sizeof(newEntry));
			Z_TYPE(newEntry) = le_pconn_struct;
			newEntry.ptr = conn_res;
			if (zend_hash_update(&EG(persistent_list), hKey, hKeyLen, (void *) &newEntry, sizeof(list_entry), NULL)==FAILURE) {
				rc = SQL_ERROR;
				/* TBD: What error to return?, for now just generic SQL_ERROR */
			}
		}
		efree(hKey);
	}

#ifdef PASE /* always change the libl if requested by options array */
	_php_db2_i5cmd_helper(conn_res);
#endif /* PASE */
	return rc;
}
/* }}} */

#ifdef CLI_DBC_SERVER_TYPE_DB2LUW
#ifdef SQL_ATTR_DECFLOAT_ROUNDING_MODE
/**
 * Function for implementation of DECFLOAT Datatype
 * 
 * Description :
 * This function retrieves the value of special register decflt_rounding
 * from the database server which signifies the current rounding mode set
 * on the server. For using decfloat, the rounding mode has to be in sync
 * on the client as well as server. Thus we set here on the client, the
 * same rounding mode as the server.
 * @return: success or failure
 * */
static void _php_db2_set_decfloat_rounding_mode_client(void* handle TSRMLS_DC)
{
	SQLCHAR decflt_rounding[20];
	SQLHANDLE hstmt;
	SQLHDBC hdbc = ((conn_handle*) handle)->hdbc;
	int rc = 0;
	int rounding_mode;
	SQLINTEGER decfloat;
	
	SQLCHAR *stmt = (SQLCHAR *)"values current decfloat rounding mode";
	
	/* Allocate a Statement Handle */
	rc = SQLAllocHandle(SQL_HANDLE_STMT, (SQLHDBC) hdbc, &hstmt);
	if (rc == SQL_ERROR) {
		_php_db2_db_check_sql_errors((SQLHDBC) hdbc, SQL_HANDLE_DBC, rc, 1,
				NULL, -1, 1 TSRMLS_CC);
		return;
	}
	rc = SQLExecDirect((SQLHSTMT)hstmt, (SQLPOINTER)stmt, SQL_NTS);
	if ( rc == SQL_ERROR ) {
		_php_db2_check_sql_errors(hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
	}
	if ( rc < SQL_SUCCESS ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
		SQLFreeHandle( SQL_HANDLE_STMT, hstmt );
		return;
	}

	rc = SQLBindCol((SQLHSTMT)hstmt, 1, SQL_C_DEFAULT, decflt_rounding, 20, NULL);
	if (rc == SQL_ERROR) {
		_php_db2_db_check_sql_errors((SQLHANDLE) hdbc, SQL_HANDLE_DBC, rc, 1,
				NULL, -1, 1 TSRMLS_CC);
		return;
	 }

	rc = SQLFetch(hstmt);
	rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	/* Now setting up the same rounding mode on the client*/
	if(strcmp(decflt_rounding, "ROUND_HALF_EVEN") == 0) {
		rounding_mode = SQL_ROUND_HALF_EVEN;
	}
	if(strcmp(decflt_rounding, "ROUND_HALF_UP") == 0) {
		rounding_mode = SQL_ROUND_HALF_UP;
	}
	if(strcmp(decflt_rounding, "ROUND_DOWN") == 0) {
		rounding_mode = SQL_ROUND_DOWN;
	}
	if(strcmp(decflt_rounding, "ROUND_CEILING") == 0) {
		rounding_mode = SQL_ROUND_CEILING;
	}
	if(strcmp(decflt_rounding, "ROUND_FLOOR") == 0) {
		rounding_mode = SQL_ROUND_FLOOR;
	}
#ifndef PASE
	rc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_DECFLOAT_ROUNDING_MODE, (SQLPOINTER)rounding_mode, SQL_NTS);
#else
	rc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_DECFLOAT_ROUNDING_MODE, (SQLPOINTER)&rounding_mode, SQL_NTS);
#endif
	
	return;
}
#endif
#endif

/* {{{ static void _php_db2_clear_conn_err_cache (TSRMLS_D)
*/
static void _php_db2_clear_conn_err_cache(TSRMLS_D)
{
	/* Clear out the cached conn messages */
	memset(IBM_DB2_G(__php_conn_err_msg), 0, DB2_MAX_ERR_MSG_LEN);
	memset(IBM_DB2_G(__php_conn_err_state), 0, SQL_SQLSTATE_SIZE + 1);
}
/* }}} */

/* {{{ static conn_handle * _php_db2_connect( INTERNAL_FUNCTION_PARAMETERS)
*/
static conn_handle * _php_db2_pconnect( INTERNAL_FUNCTION_PARAMETERS, int isPersistent )
{
	int rc;
	conn_handle *conn_res = NULL;

	_php_db2_clear_conn_err_cache(TSRMLS_C);

	rc = _php_db2_connect_helper( INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_res, isPersistent);

	if ( rc == SQL_ERROR ) {
		if (conn_res != NULL && conn_res->handle_active) {
			rc = SQLFreeHandle( SQL_HANDLE_DBC, conn_res->hdbc);
		}

		/* free memory */
		if (conn_res != NULL) {
			pefree(conn_res, 1);
		}
		conn_res = NULL;
	}
	return conn_res;
}
/* }}} */

/* {{{ proto resource db2_connect(string database, string uid, string password [, array options])
Returns a connection to a database */
PHP_FUNCTION(db2_connect)
{
	int rc;

	conn_handle *conn_res = NULL;

	_php_db2_clear_conn_err_cache(TSRMLS_C);
	
#ifdef PASE /* ini file switch all to pconnect */
	if (IBM_DB2_G(i5_all_pconnect)) {
		conn_res = _php_db2_pconnect(INTERNAL_FUNCTION_PARAM_PASSTHRU,1);
	  	if (!conn_res){
			RETVAL_FALSE;
			return;
	  	}
	  	else {
			ZEND_REGISTER_RESOURCE(return_value, conn_res,  le_pconn_struct);
	  	}
	}
	else {
#endif /* PASE */
		rc = _php_db2_connect_helper( INTERNAL_FUNCTION_PARAM_PASSTHRU, &conn_res, 0 );

		if ( rc != SQL_SUCCESS ) {
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
#ifdef PASE /* ini file switch all to pconnect */
	}
#endif /* PASE */
}
/* }}} */

/* {{{ proto resource db2_pconnect(string database_name, string username, string password [, array options])
Returns a persistent connection to a database */
PHP_FUNCTION(db2_pconnect)
{
	conn_handle *conn_res=NULL;
	conn_res=_php_db2_pconnect(INTERNAL_FUNCTION_PARAM_PASSTHRU,1);
	if (!conn_res){
		RETVAL_FALSE;
		return;
	}
	else {
		ZEND_REGISTER_RESOURCE(return_value, conn_res,  le_pconn_struct);
	}
}
/* }}} */

/* {{{ proto mixed db2_autocommit(resource connection[, bool value])
Returns or sets the AUTOCOMMIT state for a database connection */
PHP_FUNCTION(db2_autocommit)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zend_bool value;
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
			autocommit = value;
			if(autocommit != (conn_res->auto_commit)) {
#ifndef PASE
				rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)autocommit, SQL_IS_INTEGER);
#else
				rc = SQLSetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)&autocommit, SQL_IS_INTEGER);
#endif
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				}
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
	param_node *tmp_curr = NULL, *prev = stmt_res->head_cache_list, *curr = stmt_res->head_cache_list;

	while ( curr != NULL && curr->param_num != param_no ) {
		prev = curr;
		curr = curr->next;
	}

	if ( curr == NULL || curr->param_num != param_no ) {
		/* Allocate memory and make new node to be added */
		tmp_curr = (param_node *)ecalloc(1, sizeof(param_node));
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
		tmp_curr->value = 0;

		/* link pointers for the list */
		if ( prev == NULL ) {
			stmt_res->head_cache_list = tmp_curr;
		} else {
			prev->next = tmp_curr;
		}
		tmp_curr->next = curr;

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
	long param_no = 0;
	long data_type = 0;
	long precision = -1;
	long scale = 0;
	SQLSMALLINT sql_data_type = 0;
#ifdef PASE /* SQLINTEGER vs. SQLUINTEGER */
	SQLINTEGER sql_precision = 0;
#else
	SQLUINTEGER sql_precision = 0;
#endif
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
				rc = SQLDescribeParam((SQLHSTMT)stmt_res->hstmt, (SQLUSMALLINT)param_no, &sql_data_type, &sql_precision, &sql_scale, &sql_nullable);
				if ( rc == SQL_ERROR ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Describe Param Failed");
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
				if((sql_data_type == SQL_XML) && ((param_type == DB2_PARAM_OUT) || (param_type == DB2_PARAM_INOUT)))
				{
					if(precision < 0)
					{
						sql_precision = 1048576;
					}
					else
					{
						sql_precision = (SQLUINTEGER)precision;
					}
				}
#ifdef PASE /* i5/OS BIGINT to string too small */
				else if((sql_data_type == SQL_BIGINT) && ((param_type == DB2_PARAM_OUT) || (param_type == DB2_PARAM_INOUT))) 
				{
					sql_precision = 20;
				}
#endif
				/* Add to cache */
				_php_db2_add_param_cache( stmt_res, (SQLUSMALLINT)param_no, varname, varname_len, param_type, sql_data_type, sql_precision, sql_scale, sql_nullable );
				break;

			case 7:
				/* Cache param data passed */
				/* I am using a linked list of nodes here because I dont know before hand how many params are being passed in/bound. */
				/* To determine this, a call to SQLNumParams is necessary. This is take away any advantages an array would have over linked list access */
				/* Data is being copied over to the correct types for subsequent CLI call because this might cause problems on other platforms such as AIX */
				sql_data_type = (SQLSMALLINT)data_type;
				sql_precision = (SQLUINTEGER)precision;
				sql_scale = (SQLSMALLINT)scale;
				if((sql_data_type == SQL_XML) && ((param_type == DB2_PARAM_OUT) || (param_type == DB2_PARAM_INOUT)))
				{
					if(precision < 0)
					{
						sql_precision = 1048576;
					}
				}
#ifdef PASE /* i5/OS BIGINT to string too small */
				else if((sql_data_type == SQL_BIGINT) && ((param_type == DB2_PARAM_OUT) || (param_type == DB2_PARAM_INOUT))) 
				{
					sql_precision = 20;
				}
#endif
				_php_db2_add_param_cache( stmt_res, (SQLUSMALLINT)param_no, varname, varname_len, param_type, sql_data_type, sql_precision, sql_scale, sql_nullable );
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

/* {{{ static void _php_db2_close_helper( INTERNAL_FUNCTION_PARAMETERS )
*/
static void _php_db2_close_helper( INTERNAL_FUNCTION_PARAMETERS, int endpconnect )
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

#ifdef PASE /* db2_pclose - last ditch persistent close */
		if (endpconnect) {
			conn_res->flag_pconnect = 0;
		}
#endif /* PASE */

		if ( conn_res->handle_active && !conn_res->flag_pconnect ) {
			/* Disconnect from DB. If stmt is allocated, it is freed automatically */
			if (conn_res->auto_commit == 0) {
				rc = SQLEndTran(SQL_HANDLE_DBC, (SQLHDBC)conn_res->hdbc, SQL_ROLLBACK);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
			}
			rc = SQLDisconnect((SQLHDBC)conn_res->hdbc);
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				RETURN_FALSE;
			}

			rc = SQLFreeHandle( SQL_HANDLE_DBC, conn_res->hdbc);
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
				RETURN_FALSE;
			}

			conn_res->handle_active = 0;

#ifdef PASE /* db2_pclose - last ditch persistent close, but reuse zend hash */
			if (endpconnect) {
				conn_res->hdbc = 0;
				conn_res->flag_pconnect=9;
			}
#endif /* PASE */

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

/* {{{ proto bool db2_close(resource connection)
Closes a database connection */
PHP_FUNCTION(db2_close)
{
	_php_db2_close_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

#ifdef PASE /* db2_pclose - last ditch persistent close */
/* {{{ proto bool db2_pclose(resource connection)
Closes a database connection */
PHP_FUNCTION(db2_pclose)
{
	_php_db2_close_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
#endif /* PASE */


/* {{{ proto resource db2_column_privileges(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the columns and associated privileges for a table */
PHP_FUNCTION(db2_column_privileges)
{
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
	SQLCHAR *column_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len = 0;
	int owner_len = 0;
	int table_name_len = 0;
	int column_name_len = 0;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	int rc = 0;

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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5OS CatalogName=NULL. Length must be set to 0. */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				&column_name,	&column_name_len);
		rc = SQLColumnPrivileges((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			table_name,table_name_len,   	/* TableName,        NameLength3    */
			column_name,column_name_len); 	/* ColumnName,       NameLength4    */
#else /* not PASE */
		rc = SQLColumnPrivileges((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS, column_name,SQL_NTS);
#endif /* not PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
	SQLCHAR *column_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	int qualifier_len;
	int owner_len;
	int table_name_len;
	int column_name_len;
	zval *connection = NULL;
	conn_handle *conn_res;
	stmt_handle *stmt_res;
	stmt_handle *stmt_res_identity;
	int rc;
	int i;

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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* CatalogName=NULL, length =0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				&column_name,	&column_name_len);
		rc = SQLColumns((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			table_name,table_name_len,   	/* TableName,        NameLength3    */
			column_name,column_name_len); 	/* ColumnName,       NameLength4    */
#else
		rc = SQLColumns((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner, SQL_NTS, table_name, SQL_NTS, column_name, SQL_NTS);
#endif /* PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}

		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	}
}
/* }}} */

/* {{{ proto resource db2_foreign_keys(resource connection, string qualifier, string owner, string table_name, string column_name)
Returns a result set listing the foreign keys for a table */
PHP_FUNCTION(db2_foreign_keys)
{
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				NULL,		NULL);
		rc = SQLForeignKeys((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,	/* PKCatalogName=NULL, NameLength1=0 */
			owner,owner_len,      		/* PKSchemaName,       NameLength2   */
			table_name,table_name_len, 	/* PKTableName,        NameLength3   */
			NULL, 0,             		/* FKCatalogName=NULL, NameLength4=0 */
			NULL, SQL_NTS,       		/* FKSchemaName,       NameLength5   */
			NULL, SQL_NTS);      		/* FKTableName,        NameLength6   */
#else
		rc = SQLForeignKeys((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name, SQL_NTS, NULL, SQL_NTS,
						NULL, SQL_NTS, NULL, SQL_NTS);
#endif /* PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, length=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				NULL,		NULL);
		rc = SQLPrimaryKeys((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			table_name,table_name_len);   	/* TableName,        NameLength3    */
#else
		rc = SQLPrimaryKeys((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS,
						owner,SQL_NTS, table_name,SQL_NTS);
#endif /* PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *proc_name = NULL;
	SQLCHAR *column_name = NULL;
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
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id, "Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength1=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&proc_name,	&proc_name_len,
				&column_name,	&column_name_len);
		rc = SQLProcedureColumns((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			proc_name,proc_name_len,   	/* ProcName,         NameLength3    */
			column_name,column_name_len); 	/* ColumnName,       NameLength4    */
#else
		rc = SQLProcedureColumns((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, proc_name, SQL_NTS, column_name, SQL_NTS);
#endif /* PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *proc_name = NULL;
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
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id, "Connection Resource", le_conn_struct, le_pconn_struct);

		stmt_res = _db2_new_stmt_struct(conn_res);

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength1=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&proc_name,	&proc_name_len,
				NULL,		NULL);
		rc = SQLProcedures((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			proc_name,proc_name_len);   	/* ProcName,         NameLength3    */
#else
		rc = SQLProcedures((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, proc_name, SQL_NTS);
#endif /* PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength1=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				NULL,		NULL);
		rc = SQLSpecialColumns((SQLHSTMT)stmt_res->hstmt, SQL_BEST_ROWID,
			qualifier, qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner, owner_len,        	/* SchemaName,       NameLength2    */
			table_name, table_name_len,   	/* TableName,        NameLength3    */
			scope, SQL_NULLABLE); 		/* fScope,           Nullable       */
#else
		rc = SQLSpecialColumns((SQLHSTMT)stmt_res->hstmt,SQL_BEST_ROWID, qualifier, SQL_NTS,
			owner, SQL_NTS, table_name, SQL_NTS, scope, SQL_NULLABLE);
#endif /* not PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
	int argc = ZEND_NUM_ARGS();
	int qualifier_len;
	int owner_len;
	int table_name_len;
	zend_bool unique;
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
		sql_unique = unique;

		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if (rc == SQL_ERROR) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength1=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				NULL,		NULL);
		rc = SQLStatistics((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			table_name,table_name_len,   	/* TableName,        NameLength3    */
			sql_unique, 0);         	/* fUnique,          fAccuracy=0    */
#else /* not PASE */
		rc = SQLStatistics((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
			SQL_NTS, table_name, SQL_NTS, sql_unique, SQL_QUICK);
#endif /* not PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength1=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				NULL,		NULL);
		rc = SQLTablePrivileges((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			table_name,table_name_len);   	/* TableName,        NameLength3    */
#else /* not PASE */
		rc = SQLTablePrivileges((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
				SQL_NTS, table_name, SQL_NTS);
#endif /* not PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
	SQLCHAR *qualifier = NULL;
	SQLCHAR *owner = NULL;
	SQLCHAR *table_name = NULL;
	SQLCHAR *table_type = NULL;
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

#ifdef PASE /* i5os owner cannot be NULL */
	owner = "%";
#endif /* PASE */

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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
			RETURN_FALSE;
		}
#ifdef PASE /* i5/OS CatalogName=NULL, NameLength1=0 */
		_php_db2_meta_helper(
				&qualifier,	&qualifier_len,
				&owner,		&owner_len,
				&table_name,	&table_name_len,
				&table_type,	&table_type_len);
		rc = SQLTables((SQLHSTMT)stmt_res->hstmt, 
			qualifier,qualifier_len,        /* CatalogName=NULL, NameLength1=0  */
			owner,owner_len,        	/* SchemaName,       NameLength2    */
			table_name,table_name_len,   	/* TableName,        NameLength3    */
			table_type,table_type_len);	/* TableType,        NameLength4    */
#else /* not PASE */
		rc = SQLTables((SQLHSTMT)stmt_res->hstmt, qualifier, SQL_NTS, owner,
				SQL_NTS, table_name, SQL_NTS, table_type, SQL_NTS);
#endif /* not PASE */
		if (rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			efree(stmt_res);
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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETVAL_FALSE;
			return;
		} else {
			RETURN_TRUE;
		}
	}
}
/* }}} */

/* {{{ static int _php_db2_do_prepare(SQLHANDLE hdbc, string stmt_string, stmt_handle *stmt_res, int stmt_string_len, zval *options TSRMLS_DC)
*/
static int _php_db2_do_prepare(SQLHANDLE hdbc, char* stmt_string, stmt_handle *stmt_res, int stmt_string_len, zval *options TSRMLS_DC)
{
	int rc;
	SQLINTEGER vParam;

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
	rc = SQLPrepare((SQLHSTMT)stmt_res->hstmt, (SQLCHAR*)stmt_string, (SQLINTEGER)stmt_string_len);
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
	SQLINTEGER vParam;

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
		
		/* alloc handle and return only if it errors */
		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &(stmt_res->hstmt));
		if ( rc < SQL_SUCCESS ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}

		if (options != NULL) {
			rc = _php_db2_parse_options( options, SQL_HANDLE_STMT, stmt_res TSRMLS_CC );
			if ( rc == SQL_ERROR ) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Options Array must have string indexes");
			}
		}

		rc = SQLExecDirect((SQLHSTMT)stmt_res->hstmt, stmt_string , stmt_string_len);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
		if ( rc < SQL_SUCCESS ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
			SQLFreeHandle( SQL_HANDLE_STMT, stmt_res->hstmt );
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
            /* Free any cursors that might have been allocated in a previous call to SQLExecute */
            SQLFreeStmt((SQLHSTMT)stmt_res->hstmt, SQL_CLOSE);
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
		rc = _php_db2_do_prepare(conn_res->hdbc, stmt_string, stmt_res, stmt_string_len, options TSRMLS_CC);
		if ( rc < SQL_SUCCESS ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Prepare Failed");
			efree(stmt_res);
			RETURN_FALSE;
		}

		ZEND_REGISTER_RESOURCE(return_value, stmt_res, le_stmt_struct);
	}
}
/* }}} */

/* {{{ static param_node* _php_db2_build_list( stmt_res, param_no, data_type, precision, scale, nullable )
*/
static param_node* _php_db2_build_list( stmt_handle *stmt_res, int param_no, SQLSMALLINT data_type, SQLUINTEGER precision, SQLSMALLINT scale, SQLSMALLINT nullable )
{
	param_node *tmp_curr = NULL, *curr = stmt_res->head_cache_list, *prev = NULL;

	/* Allocate memory and make new node to be added */
	tmp_curr = (param_node *)ecalloc(1, sizeof(param_node));
	/* assign values */
	tmp_curr->data_type = data_type;
	tmp_curr->param_size = precision;
	tmp_curr->nullable = nullable;
	tmp_curr->scale = scale;
	tmp_curr->param_num = param_no;
	tmp_curr->file_options = SQL_FILE_READ;
	tmp_curr->param_type = DB2_PARAM_IN;
	tmp_curr->long_value = 0;
	tmp_curr->value = 0;

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
static int _php_db2_bind_data( stmt_handle *stmt_res, param_node *curr, zval **bind_data TSRMLS_DC)
{
	int rc;
	SQLSMALLINT valueType;
	SQLPOINTER  paramValuePtr;
	int nullterm = 0;

	/* Clean old zval value and create a new one */
	if( curr->value != 0 && curr->param_type != DB2_PARAM_OUT && curr->param_type != DB2_PARAM_INOUT )
		zval_ptr_dtor(&curr->value);
	MAKE_STD_ZVAL(curr->value);

	switch ( curr->data_type ) {
#ifndef PASE /* i5/OS int not big enough */
		case SQL_BIGINT:
#endif
		case SQL_SMALLINT:
		case SQL_INTEGER:
		case SQL_REAL:
		case SQL_FLOAT:
		case SQL_DOUBLE:
			break;

#ifdef PASE /* i5/OS int not big enough */
		case SQL_BIGINT:
#endif
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		case SQL_GRAPHIC:
		case SQL_VARGRAPHIC:
#ifndef PASE /* i5/OS SQL_LONGVARCHAR is SQL_VARCHAR */
		case SQL_LONGVARCHAR:
		case SQL_WLONGVARCHAR:
		case SQL_LONGVARGRAPHIC:
#endif /* PASE */
		case SQL_DECIMAL:
		case SQL_NUMERIC:
		case SQL_DECFLOAT:
		case SQL_CLOB:
#ifdef PASE
		case SQL_UTF8_CHAR:
#endif
		case SQL_XML:
		case SQL_DBCLOB:
			nullterm = 1;
#ifdef PASE /* i5/OS incompatible v6r1 change */
		case SQL_VARBINARY_V6:
		case SQL_BINARY_V6:
#endif /* PASE */
		case SQL_BINARY:
#ifndef PASE /* i5/OS SQL_LONGVARBINARY is SQL_VARBINARY */
		case SQL_LONGVARBINARY:
#endif /* PASE */
		case SQL_VARBINARY:
		case SQL_BLOB:
			/* ADC: test 145/many need NULL special handling (LUW and i5/OS) */
			if (Z_TYPE_PP(bind_data) == IS_NULL){
				break;
			}
			/* make sure copy in is a string (Z_STRVAL_PP(bind_data) && ) */
			if (*bind_data && Z_TYPE_PP(bind_data) != IS_STRING) {
				convert_to_string(*bind_data);
			}
			if (curr->param_type == DB2_PARAM_OUT || curr->param_type == DB2_PARAM_INOUT) {
				int origlen = Z_STRLEN_PP(bind_data);
				if (Z_STRLEN_PP(bind_data) < curr->param_size+nullterm) {
					Z_STRVAL_PP(bind_data) = erealloc(Z_STRVAL_PP(bind_data), curr->param_size+nullterm);
					if (Z_STRVAL_PP(bind_data) == NULL ) {
						return SQL_ERROR;
					}
#ifndef PASE
					if (curr->param_type == DB2_PARAM_INOUT)
#endif
						memset(Z_STRVAL_PP(bind_data)+origlen,0x20, curr->param_size-origlen);
					if (nullterm) Z_STRVAL_PP(bind_data)[curr->param_size] = '\0';
						Z_STRLEN_PP(bind_data) = curr->param_size;
				}
			}
#ifdef PASE /* zero length valueType = SQL_C_CHAR bad for i5/OS SQLBindParameter */
			else if (Z_STRLEN_PP(bind_data) == 0) {
				Z_TYPE_PP(bind_data) = IS_STRING;
				Z_STRVAL_PP(bind_data) = erealloc(Z_STRVAL_PP(bind_data), curr->param_size+nullterm);
				memset(Z_STRVAL_PP(bind_data), 0x20, curr->param_size+nullterm);
				if (nullterm) {
					Z_STRVAL_PP(bind_data)[curr->param_size] = '\0';
				}
				Z_STRLEN_PP(bind_data) = curr->param_size;
			}
			/* i5/OS int not big enough (zero fill left justified signed number) */
			if (curr->data_type == SQL_BIGINT) {
				char *front,*back;
				front=Z_STRVAL_PP(bind_data);
				back=Z_STRVAL_PP(bind_data)+Z_STRLEN_PP(bind_data)-1;
				while (*front && front<=back) {
					if (*front != '-' && *front != ' ') {
						*back=*front;
						back--;
						if (front<back){
							*front = '0';
						}
					}
					if (*front == ' ') {
						*front = '0';
					}
					front++;
				}
			}
#endif /* PASE */
			break;
		case SQL_TYPE_DATE:
		case SQL_DATETIME:
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIMESTAMP:
			if (curr->param_type == DB2_PARAM_OUT || curr->param_type == DB2_PARAM_INOUT) {
				int origlen = Z_STRLEN_PP(bind_data);
				if (Z_STRLEN_PP(bind_data) < curr->param_size + 1) {
					Z_STRVAL_PP(bind_data) = erealloc(Z_STRVAL_PP(bind_data), curr->param_size + 1);
					if (Z_STRVAL_PP(bind_data) == NULL ) {
						return SQL_ERROR;
					}
					if (curr->param_type == DB2_PARAM_INOUT)
						memset(Z_STRVAL_PP(bind_data) + origlen,0x20, curr->param_size-origlen);
					Z_STRVAL_PP(bind_data)[curr->param_size] = '\0';
					Z_STRLEN_PP(bind_data) = curr->param_size;
				}
			}
#ifdef PASE /* zero length valueType = SQL_C_CHAR bad for i5/OS SQLBindParameter */
			else if (Z_STRLEN_PP(bind_data) == 0) {
				Z_STRVAL_PP(bind_data) = erealloc(Z_STRVAL_PP(bind_data), curr->param_size + 1);
				memset(Z_STRVAL_PP(bind_data), 0x20, curr->param_size + 1);
				Z_STRVAL_PP(bind_data)[curr->param_size] = '\0';
				Z_STRLEN_PP(bind_data) = curr->param_size;
			}
#endif /* PASE */
			break;


		default:
			return SQL_ERROR;
	}

	/* copy data over from bind_data */
	*(curr->value) = **bind_data;
    zval_copy_ctor(curr->value);
	INIT_PZVAL(curr->value);

	/* Have to use SQLBindFileToParam if PARAM is type DB2_PARAM_FILE */
	if ( curr->param_type == DB2_PARAM_FILE) {
		/* Only string types can be bound */
		if ( Z_TYPE_PP(bind_data) != IS_STRING) {
			return SQL_ERROR;
		}

		curr->bind_indicator = 0;
		/* Bind file name string */
		curr->short_strlen = (SQLSMALLINT) ((curr->value)->value.str.len);
		rc = SQLBindFileToParam((SQLHSTMT)stmt_res->hstmt, curr->param_num,
				curr->data_type, (SQLCHAR *)((curr->value)->value.str.val),
				(SQLSMALLINT *)&curr->short_strlen, &(curr->file_options),
				Z_STRLEN_P(curr->value), &(curr->bind_indicator));
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}

		return rc;
	}

	switch(Z_TYPE_PP(bind_data)) {
		case IS_BOOL:
		case IS_LONG:
			curr->long_value = (SQLINTEGER)(curr->value)->value.lval;
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
					curr->param_type, SQL_C_LONG, curr->data_type,
					curr->param_size, curr->scale, &curr->long_value, 0, NULL);
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			}
			break;

		case IS_DOUBLE:
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
					curr->param_type, SQL_C_DOUBLE, curr->data_type, curr->param_size,
					curr->scale, &((curr->value)->value.dval), 0, NULL);
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			}
			break;

		case IS_STRING:
			switch ( curr->data_type ) {
				case SQL_CLOB:
				case SQL_DBCLOB:
					if (curr->param_type == DB2_PARAM_OUT || curr->param_type == DB2_PARAM_INOUT) {
						curr->bind_indicator = (curr->value)->value.str.len;
						valueType = SQL_C_CHAR;
						paramValuePtr = (SQLPOINTER)((curr->value)->value.str.val);
					} else {
						curr->bind_indicator = SQL_DATA_AT_EXEC;
						valueType = SQL_C_CHAR;
						/* The correct dataPtr will be set during SQLPutData with the len from this struct */
						paramValuePtr = (SQLPOINTER)&((curr->value)->value);
					}
					break;

				case SQL_BLOB:
					if (curr->param_type == DB2_PARAM_OUT || curr->param_type == DB2_PARAM_INOUT) {
						curr->bind_indicator = (curr->value)->value.str.len;
#ifdef PASE /* i5/OS V6R1 incompatible change */
						if (is_i5os_classic){
							valueType = SQL_C_BINARY;
						} else {
							valueType = SQL_C_BINARY_V6;
						}
#else
						valueType = SQL_C_BINARY;
#endif /* PASE */

						paramValuePtr = (SQLPOINTER)((curr->value)->value.str.val);
					} else {
						curr->bind_indicator = SQL_DATA_AT_EXEC;
#ifdef PASE /* i5/OS V6R1 incompatible change */
						if (is_i5os_classic){
							valueType = SQL_C_BINARY;
						} else {
							valueType = SQL_C_BINARY_V6;
						}
#else
						valueType = SQL_C_BINARY;
#endif /* PASE */
						paramValuePtr = (SQLPOINTER)&((curr->value)->value);
					}
					break;

#ifdef PASE /* i5/OS incompatible v6r1 change */
				case SQL_VARBINARY_V6:
				case SQL_BINARY_V6:
#endif /* PASE */
				case SQL_BINARY:
#ifndef PASE /* i5/OS SQL_LONGVARBINARY is SQL_VARBINARY */
				case SQL_LONGVARBINARY:
#endif /* PASE */
				case SQL_VARBINARY:
				case SQL_XML:
					/* account for bin_mode settings as well */
					curr->bind_indicator = (curr->value)->value.str.len;
#ifdef PASE /* i5/OS V6R1 incompatible change */
					if (is_i5os_classic){
						valueType = SQL_C_BINARY;
					} else {
						valueType = SQL_C_BINARY_V6;
					}
#else
					valueType = SQL_C_BINARY;
#endif /* PASE */
					paramValuePtr = (SQLPOINTER)((curr->value)->value.str.val);
					break;
#ifdef PASE /* i5/OS should be SQL_NTS to avoid extra spaces */
				case SQL_VARCHAR:
					valueType = SQL_C_CHAR;
					curr->bind_indicator = SQL_NTS;
					paramValuePtr = (SQLPOINTER)((curr->value)->value.str.val);
					break;
				case SQL_BIGINT:
					valueType = SQL_C_CHAR;
					curr->bind_indicator = SQL_NTS; /* 20 */
					paramValuePtr = (SQLPOINTER)((curr->value)->value.str.val);
					break;
#endif /* PASE */
				/* This option should handle most other types such as DATE, VARCHAR etc */
				default:
					valueType = SQL_C_CHAR;
					curr->bind_indicator = (curr->value)->value.str.len;
					paramValuePtr = (SQLPOINTER)((curr->value)->value.str.val);
			}

			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
					curr->param_type, valueType, curr->data_type, curr->param_size,
					curr->scale, paramValuePtr, Z_STRLEN_P(curr->value)+1, &(curr->bind_indicator));
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			}
			break;

		case IS_NULL:
			Z_LVAL_P(curr->value) = SQL_NULL_DATA;
			rc = SQLBindParameter(stmt_res->hstmt, curr->param_num,
					curr->param_type, SQL_C_DEFAULT, curr->data_type, curr->param_size,
					curr->scale, &(curr->value), 0, &((curr->value)->value.lval));
			if ( rc == SQL_ERROR ) {
				_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			}
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
	SQLUSMALLINT param_no;
	SQLSMALLINT data_type;
#ifdef PASE /* SQLINTEGER vs SQLUINTEGER */
	SQLINTEGER precision;
#else
	SQLUINTEGER precision;
#endif
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
				rc = _php_db2_bind_data( stmt_res, curr, bind_data TSRMLS_CC);
				if ( rc == SQL_ERROR ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Binding Error 1");
					return rc;
				}
				curr = curr->next;
			} else if ( zend_hash_find(&EG(symbol_table), curr->varname,
						strlen(curr->varname)+1, (void **) &bind_data ) != FAILURE ) {
				rc = _php_db2_bind_data( stmt_res, curr, bind_data TSRMLS_CC);
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
					_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
					return rc;
				}

				curr = _php_db2_build_list( stmt_res, param_no, data_type, precision, scale, nullable );

				rc = _php_db2_bind_data( stmt_res, curr, data TSRMLS_CC);
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
					rc = _php_db2_bind_data( stmt_res, curr, data TSRMLS_CC);
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
#ifdef PASE /* i5 LUW bug -- this is an output for SQLParamData */	
	SQLPOINTER valuePtr=NULL;
#else
	SQLPOINTER valuePtr;
#endif

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

	if (!stmt) {
		return;
	}

	ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

	/* Free any cursors that might have been allocated in a previous call to SQLExecute */
	SQLFreeStmt((SQLHSTMT)stmt_res->hstmt, SQL_CLOSE);

	/* This ensures that each call to db2_execute start from scratch */
	stmt_res->current_node = stmt_res->head_cache_list;

#ifdef PASE /* i5 fetch not always called need to reset (probably LUW too) */	
	_php_db2_init_error_info(stmt_res);
#endif /* PASE */

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
			_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_TRUE;
	}

	/* Execute Stmt -- All parameters bound */
	rc = SQLExecute((SQLHSTMT)stmt_res->hstmt);
	if ( rc == SQL_ERROR ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		RETVAL_FALSE;
	}
#ifdef PASE /* i5 warnings ignored -- but maybe only serious errors desired (probably LUW too) */	
	if ( rc == SQL_SUCCESS_WITH_INFO ) {
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, SQL_ERROR, 1, NULL, -1, 1 TSRMLS_CC);
	}
#endif /* PASE */
	if ( rc == SQL_NEED_DATA ) {
		while ( (rc = (SQLParamData((SQLHSTMT)stmt_res->hstmt, (SQLPOINTER *)&valuePtr))) == SQL_NEED_DATA ) {
			/* passing data value for a parameter */
			rc = SQLPutData((SQLHSTMT)stmt_res->hstmt, (SQLPOINTER)(((zvalue_value*)valuePtr)->str.val), ((zvalue_value*)valuePtr)->str.len);
			if ( rc == SQL_ERROR ) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Sending data failed");
				_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
				RETVAL_FALSE;
			}
		}
		if ( rc == SQL_ERROR ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Statement Execute Failed");
			_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETVAL_FALSE;
		}
	}

	/* cleanup dynamic bindings if present */
	if ( bind_params == 1 ) {
		/* Free param cache list */
		curr_ptr = stmt_res->head_cache_list;
		prev_ptr = stmt_res->head_cache_list;

		while (curr_ptr != NULL) {
			curr_ptr = curr_ptr->next;

			/* Free Values */
			if (prev_ptr->value != NULL) {
				if ( Z_TYPE_P(prev_ptr->value) == IS_STRING ) {
					if((prev_ptr->value)->value.str.val != NULL || (prev_ptr->value)->value.str.len != 0) {
						efree((prev_ptr->value)->value.str.val);
					}
				}

				if( prev_ptr->param_type != DB2_PARAM_OUT && prev_ptr->param_type != DB2_PARAM_INOUT ){
					efree(prev_ptr->value);
				}
			}
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
					if( Z_TYPE_P( tmp_curr->value ) == IS_STRING &&
						(tmp_curr->bind_indicator != SQL_NULL_DATA &&
						 tmp_curr->bind_indicator != SQL_NO_TOTAL )) {
						/*
							if the length of the string out parameter is returned
							then correct the length of the corresponding php variable
						*/
						if(Z_STRLEN_P(tmp_curr->value) < tmp_curr->bind_indicator) {
							tmp_curr->bind_indicator = Z_STRLEN_P(tmp_curr->value);
						}
						tmp_curr->value->value.str.val[tmp_curr->bind_indicator] = 0;
#ifdef PASE /* i5/OS both INOUT and OUT */
						if (tmp_curr->param_type == DB2_PARAM_INOUT || tmp_curr->param_type == DB2_PARAM_OUT) {
							int ib = tmp_curr->bind_indicator-1;
							for (;ib && Z_STRVAL_P(tmp_curr->value)[ib] == 0x20 || Z_STRVAL_P(tmp_curr->value)[ib] == 0x40; ib--){
								Z_STRVAL_P(tmp_curr->value)[ib] = '\0';
							}
						}
#else
						if (tmp_curr->param_type == DB2_PARAM_INOUT) {
							int ib = tmp_curr->bind_indicator-1;
							for (;ib && Z_STRVAL_P(tmp_curr->value)[ib] == 0x20; ib--){
								Z_STRVAL_P(tmp_curr->value)[ib] = '\0';
							}
						}
#endif
						Z_STRLEN_P(tmp_curr->value) = strlen(Z_STRVAL_P(tmp_curr->value));

					}
					else if (Z_TYPE_P(tmp_curr->value) == IS_LONG || Z_TYPE_P(tmp_curr->value) == IS_BOOL) {
						/* bind in the value of long_value instead */
						tmp_curr->value->value.lval = (long)tmp_curr->long_value;
					}
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
		return_str = (char*)ecalloc(1, DB2_MAX_ERR_MSG_LEN);

		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, -1, 0, return_str, DB2_ERRMSG, conn_res->errormsg_recno_tracker TSRMLS_CC);
		if(conn_res->errormsg_recno_tracker - conn_res->error_recno_tracker >= 1) {
			conn_res->error_recno_tracker = conn_res->errormsg_recno_tracker;
		}
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
		return_str = (char*)ecalloc(1, DB2_MAX_ERR_MSG_LEN);

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
		return_str = (char*)ecalloc(1, SQL_SQLSTATE_SIZE + 1);

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
		return_str = (char*)ecalloc(1, SQL_SQLSTATE_SIZE + 1);

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
		new_stmt_res = (stmt_handle *)ecalloc(1, sizeof(stmt_handle));
		new_stmt_res->s_bin_mode = stmt_res->s_bin_mode;
		new_stmt_res->cursor_type = stmt_res->cursor_type;
		new_stmt_res->s_case_mode = stmt_res->s_case_mode;
#ifdef PASE /* i5 override php.ini */
		new_stmt_res->s_i5_allow_commit = stmt_res->s_i5_allow_commit;
		new_stmt_res->s_i5_dbcs_alloc   = stmt_res->s_i5_dbcs_alloc;
#endif /* PASE */
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
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
		}

		RETURN_LONG(count);
	}
}
/* }}} */

/* {{{ static int _php_db2_get_column_by_name(stmt_handle *stmt_res, char *col_name, int col TSRMLS_DC)
	*/
static int _php_db2_get_column_by_name(stmt_handle *stmt_res, char *col_name, int col TSRMLS_DC)
{
	int i;
	/* get column header info*/
	if ( stmt_res->column_info == NULL ) {
		if (_php_db2_get_result_set_info(stmt_res TSRMLS_CC)<0) {
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
		if (strcmp((char *)stmt_res->column_info[i].name,col_name) == 0) {
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
	col = _php_db2_get_column_by_name(stmt_res,col_name, col TSRMLS_CC);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	RETURN_STRING((char *)stmt_res->column_info[col].name,1);
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
	col = _php_db2_get_column_by_name(stmt_res,col_name, col TSRMLS_CC);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	rc = SQLColAttributes((SQLHSTMT)stmt_res->hstmt,(SQLSMALLINT)col+1,
			SQL_DESC_DISPLAY_SIZE,NULL,0, NULL,&colDataDisplaySize);
	if ( rc < SQL_SUCCESS ) {
		_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
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
	col = _php_db2_get_column_by_name(stmt_res,col_name, col TSRMLS_CC);
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
	col = _php_db2_get_column_by_name(stmt_res,col_name, col TSRMLS_CC);
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
	col = _php_db2_get_column_by_name(stmt_res,col_name, col TSRMLS_CC);
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
	col = _php_db2_get_column_by_name(stmt_res,col_name, col TSRMLS_CC);
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
		case SQL_DECFLOAT:
			str_val = "real";
			break;
		case SQL_CLOB:
			str_val = "clob";
			break;
		case SQL_DBCLOB:
			str_val = "dbclob";
			break;
		case SQL_BLOB:
			str_val = "blob";
			break;
		case SQL_XML:
			str_val = "xml";
			break;
		case SQL_TYPE_DATE:
			str_val = "date";
			break;
		case SQL_TYPE_TIME:
			str_val = "time";
			break;
	    case SQL_DATETIME:
			str_val = "datetime";
			break;
		case SQL_TYPE_TIMESTAMP:
			str_val = "timestamp";
			break;
		default:
			str_val = "string";
			break;
	}
	RETURN_STRING(str_val, 1);
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
	int col = -1;
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
	col = _php_db2_get_column_by_name(stmt_res, col_name, col TSRMLS_CC);
	if ( col < 0 ) {
		RETURN_FALSE;
	}
	rc = SQLColAttributes((SQLHSTMT)stmt_res->hstmt,(SQLSMALLINT)col + 1,
			SQL_DESC_LENGTH, NULL, 0, NULL, &colDataSize);
	if ( rc != SQL_SUCCESS ) {
		_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
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
#ifdef PASE
	RETURN_LONG(stmt_res->cursor_type);
#else
	RETURN_LONG(stmt_res->cursor_type != DB2_FORWARD_ONLY);
#endif
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
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
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
/*		 
     This function implementation had been deprecated due to the possibility of		 
     double free'ing of statement resources.	 
     Testcase:	 
*/
	RETURN_TRUE;
}
/* }}} */

/* {{{ static RETCODE _php_db2_get_data(stmt_handle *stmt_res, int col_num, short ctype, void *buff, int in_length, SQLINTEGER *out_length TSRMLS_DC) */
static RETCODE _php_db2_get_data(stmt_handle *stmt_res, SQLUSMALLINT col_num, SQLSMALLINT ctype, SQLPOINTER buff, SQLLEN in_length, SQLLEN *out_length TSRMLS_DC)
{
	RETCODE rc=SQL_SUCCESS;

	rc = SQLGetData((SQLHSTMT)stmt_res->hstmt, col_num, ctype, buff, in_length, out_length);
	if ( rc == SQL_ERROR ) {
		_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
	}

	return rc;
}
/* }}} */

/* {{{ static RETCODE _php_db2_get_length(stmt_handle* stmt_res, SQLUSMALLINT col_num, SQLINTEGER *sLength TSRMLS_DC) */
static RETCODE _php_db2_get_length(stmt_handle* stmt_res, SQLUSMALLINT col_num, SQLINTEGER *sLength TSRMLS_DC)
{
	RETCODE rc=SQL_SUCCESS;
	SQLHANDLE new_hstmt;

	rc = SQLAllocHandle(SQL_HANDLE_STMT, stmt_res->hdbc, &new_hstmt);
	if ( rc < SQL_SUCCESS ) {
		_php_db2_check_sql_errors(stmt_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		return SQL_ERROR;
	}

	rc = SQLGetLength((SQLHSTMT)new_hstmt, stmt_res->column_info[col_num-1].loc_type,
				stmt_res->column_info[col_num-1].lob_loc, sLength,
				&stmt_res->column_info[col_num-1].loc_ind);
	if ( rc == SQL_ERROR ) {
		/* adc -- bug stmt_res not right changed to new_hstmt */
		_php_db2_check_sql_errors((SQLHSTMT)new_hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
	}
#ifdef PASE /* i5/OS special DBCS */
	if (*sLength != SQL_NULL_DATA){
		if (stmt_res->s_i5_dbcs_alloc) {
			switch (stmt_res->column_info[col_num-1].type) {
				case SQL_CHAR:
				case SQL_VARCHAR:
				case SQL_CLOB:
				case SQL_DBCLOB:
				case SQL_UTF8_CHAR:
				case SQL_WCHAR:
				case SQL_WVARCHAR:
				case SQL_GRAPHIC:
				case SQL_VARGRAPHIC:
					*sLength = *sLength * 6;
		    }
		}
	}
#endif /*  PASE */

	SQLFreeHandle(SQL_HANDLE_STMT, new_hstmt);

	return rc;
}
/* }}} */

/* {{{ static RETCODE _php_db2_get_data2(stmt_handle *stmt_res, int col_num, short ctype, void *buff, int read_length, int buff_length, SQLINTEGER *out_length TSRMLS_DC) */
static RETCODE _php_db2_get_data2(stmt_handle *stmt_res, SQLUSMALLINT col_num, SQLSMALLINT ctype, SQLPOINTER buff, SQLLEN read_length, SQLLEN buff_length, SQLINTEGER *out_length TSRMLS_DC)
{
	RETCODE rc=SQL_SUCCESS;
	SQLHANDLE new_hstmt;
	SQLSMALLINT locType = ctype;
	SQLSMALLINT targetCType = ctype;
#ifdef PASE /* i5/OS CLI SQLGetSubString zero length error (PHP scripts loops fail wrongly) */
	if (!read_length) {
		*out_length=1;
		return SQL_SUCCESS;
	}
#endif /* PASE */
	rc = SQLAllocHandle(SQL_HANDLE_STMT, stmt_res->hdbc, &new_hstmt);
	if ( rc < SQL_SUCCESS ) {
		_php_db2_check_sql_errors(stmt_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		return SQL_ERROR;
	}
	rc = SQLGetSubString((SQLHSTMT)new_hstmt, stmt_res->column_info[col_num-1].loc_type,
				stmt_res->column_info[col_num-1].lob_loc, 1, read_length, targetCType,
				buff, buff_length, out_length, &stmt_res->column_info[col_num-1].loc_ind);
	if ( rc == SQL_ERROR ) {
		/* adc -- bug stmt_res not right changed to new_hstmt */
		_php_db2_check_sql_errors((SQLHSTMT)new_hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
	}

	SQLFreeHandle(SQL_HANDLE_STMT, new_hstmt);

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
	SQLSMALLINT col_num = 0;
	RETCODE rc = 0;
	void	*out_ptr = NULL;
	char	*out_char_ptr = NULL;
	SQLINTEGER in_length = 0, out_length=-10; /* Initialize out_length to some meaningless value */
	SQLSMALLINT column_type, lob_bind_type= SQL_C_BINARY;
	SQLDOUBLE double_val = 0;
	SQLINTEGER long_val = 0;
#ifdef PASE /* hack */
	char i5oshack;
#endif /* PASE */
#ifdef PASE /* i5/OS V6R1 incompatible change */
	if (is_i5os_classic){
		lob_bind_type= SQL_C_BINARY;
	} else {
		lob_bind_type= SQL_C_BINARY_V6;
	}
#endif /* PASE */

	if (zend_parse_parameters(argc TSRMLS_CC, "rz", &stmt, &column) == FAILURE) {
		return;
	}

	if (stmt) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

		if(Z_TYPE_P(column) == IS_STRING) {
			col_num = _php_db2_get_column_by_name(stmt_res, Z_STRVAL_P(column), -1 TSRMLS_CC);
		} else {
			col_num = (SQLSMALLINT)Z_LVAL_P(column);
		}

		/* get column header info*/
		if ( stmt_res->column_info == NULL ) {
			if (_php_db2_get_result_set_info(stmt_res TSRMLS_CC)<0) {
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
#ifdef PASE
			case SQL_UTF8_CHAR:
#endif
			case SQL_WCHAR:
			case SQL_WVARCHAR:
			case SQL_GRAPHIC:
			case SQL_VARGRAPHIC:
#ifndef PASE /* i5/OS SQL_DBCLOB */
			case SQL_DBCLOB:
#endif /* not PASE */
#ifndef PASE /* i5/OS SQL_LONGVARCHAR is SQL_VARCHAR */
			case SQL_LONGVARCHAR:
			case SQL_WLONGVARCHAR:
			case SQL_LONGVARGRAPHIC:
#endif /* PASE */
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
			case SQL_TYPE_TIMESTAMP:
			case SQL_DATETIME:
			case SQL_BIGINT:
			case SQL_DECIMAL:
			case SQL_NUMERIC:
			case SQL_DECFLOAT:

#ifdef PASE /* i5/OS example of "too small" allocation convert problem SQL_C_CHAR */
				switch(column_type) {
					case SQL_TYPE_DATE:
					case SQL_TYPE_TIME:
					case SQL_TYPE_TIMESTAMP:
					/* case SQL_DATETIME: this one is CHAR (ok i think) */
					case SQL_BIGINT:
					case SQL_DECIMAL:
					case SQL_NUMERIC:
						rc = SQLColAttributes((SQLHSTMT)stmt_res->hstmt,
								(SQLSMALLINT)col_num+1,
								SQL_DESC_DISPLAY_SIZE,
								NULL, 0, NULL,
								&stmt_res->column_info[col_num].size);
						if ( rc == SQL_ERROR ) {
							RETURN_FALSE;
						}
					default:
						break;
				}
#else /* not PASE */

				if (column_type == SQL_DECIMAL || column_type == SQL_NUMERIC)
					in_length = stmt_res->column_info[col_num].size + stmt_res->column_info[col_num].scale + 2 + 1;
				else
#endif /* not PASE */
				in_length = stmt_res->column_info[col_num].size+1;
				if(column_type == SQL_BIGINT) {
					in_length++;
				}
				out_ptr = (SQLPOINTER)ecalloc(1, in_length);
				if ( out_ptr == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory");
					RETURN_FALSE;
				}
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_CHAR, out_ptr, in_length, &out_length TSRMLS_CC);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					efree(out_ptr);
					RETURN_NULL();
				} else {
					RETVAL_STRING((char*)out_ptr, 1);
					efree(out_ptr);
				}
				break;

			case SQL_SMALLINT:
			case SQL_INTEGER:
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_LONG, (SQLPOINTER)&long_val, sizeof(long_val), &out_length TSRMLS_CC);
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
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_DOUBLE, (SQLPOINTER)&double_val, sizeof(double_val), &out_length TSRMLS_CC);
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
#ifdef PASE /* i5/OS SQL_DBCLOB */
			case SQL_DBCLOB:
#endif /* PASE */
				lob_bind_type= SQL_C_CHAR;
				if (column_type==SQL_CLOB) {
					stmt_res->column_info[col_num].loc_type = SQL_CLOB_LOCATOR;
					rc = _php_db2_get_data(stmt_res, col_num+1, SQL_CLOB_LOCATOR,
						(SQLPOINTER)&stmt_res->column_info[col_num].lob_loc,
						sizeof(stmt_res->column_info[col_num].lob_loc), &out_length TSRMLS_CC);
				} else {
					stmt_res->column_info[col_num].loc_type = SQL_DBCLOB_LOCATOR;
					rc = _php_db2_get_data(stmt_res, col_num+1, SQL_DBCLOB_LOCATOR,
						(SQLPOINTER)&stmt_res->column_info[col_num].lob_loc,
						sizeof(stmt_res->column_info[col_num].lob_loc), &out_length TSRMLS_CC);
				}
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}
				rc=_php_db2_get_length(stmt_res, col_num+1, &in_length TSRMLS_CC);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (in_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}
				out_char_ptr = (char*)ecalloc(1, in_length+1);
				if ( out_char_ptr == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
					RETURN_FALSE;
				}
				rc = _php_db2_get_data2(stmt_res, col_num+1, SQL_C_CHAR, (void*)out_char_ptr, in_length, in_length+1, &out_length TSRMLS_CC);
				if (rc == SQL_ERROR) {
					RETURN_FALSE;
				}
				RETURN_STRINGL(out_char_ptr, out_length, 0);
				break;
			case SQL_BLOB:
#ifdef PASE /* i5/OS incompatible v6r1 change */
			case SQL_VARBINARY_V6:
			case SQL_BINARY_V6:
#endif /* PASE */
			case SQL_BINARY:
#ifndef PASE /* i5/OS SQL_LONGVARBINARY is SQL_VARBINARY */
			case SQL_LONGVARBINARY:
#endif /* PASE */
			case SQL_VARBINARY:
#ifndef PASE
				stmt_res->column_info[col_num].loc_type = SQL_BLOB_LOCATOR;
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_BLOB_LOCATOR,
									   (SQLPOINTER)&stmt_res->column_info[col_num].lob_loc,
									   sizeof(stmt_res->column_info[col_num].lob_loc), &out_length TSRMLS_CC);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}

				switch (stmt_res->s_bin_mode) {
					case DB2_PASSTHRU:
						RETVAL_EMPTY_STRING();
						break;
					/* returns here */
					case DB2_CONVERT:
#ifndef PASE /* i5/OS BINARY SQL_C_CHAR errors */
						in_length *= 2;
						lob_bind_type = SQL_C_CHAR;
#endif /* PASE */
					/* fall-through */
					case DB2_BINARY:
					default:
						break;
				}
				rc=_php_db2_get_length(stmt_res, col_num+1, &in_length TSRMLS_CC);
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
#ifndef PASE /* i5/OS BINARY SQL_C_CHAR errors */
						in_length *= 2;
						lob_bind_type = SQL_C_CHAR;
#endif /* PASE */
					/* fall-through */
					case DB2_BINARY:
						out_ptr = (SQLPOINTER)ecalloc(1, in_length);
						if ( out_ptr == NULL ) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
							RETURN_FALSE;
						}
						rc = _php_db2_get_data2(stmt_res, col_num+1, lob_bind_type, (char *)out_ptr, in_length, in_length, &out_length TSRMLS_CC);
						if (rc == SQL_ERROR) {
							RETURN_FALSE;
						}
						RETVAL_STRINGL((char*)out_ptr,out_length, 0);
					default:
						break;
				}
				break;
#endif /* not  PASE */
			case SQL_XML:
#ifdef PASE /* PASE hack */
				/* i5/OS get one byte hack because in_length=0 not work
                                 */
				out_char_ptr = &i5oshack; in_length=1;
				rc = _php_db2_get_data(stmt_res, col_num+1, lob_bind_type, out_char_ptr, in_length, &in_length TSRMLS_CC);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}
				out_char_ptr = (SQLPOINTER)ecalloc(1, in_length+2);
				out_char_ptr[in_length+1] = '\0';
				if ( out_char_ptr == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory");
					RETURN_FALSE;
				}
				rc = _php_db2_get_data(stmt_res, col_num+1, lob_bind_type, out_char_ptr+1, in_length, &out_length TSRMLS_CC);
				out_char_ptr[0] = i5oshack;
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (out_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}
				RETVAL_STRINGL((char*)out_char_ptr,out_length, 0);
#else /* not  PASE */
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_BINARY, NULL, 0, (SQLINTEGER *)&in_length TSRMLS_CC);
				if ( rc == SQL_ERROR ) {
					RETURN_FALSE;
				}
				if (in_length == SQL_NULL_DATA) {
					RETURN_NULL();
				}
				out_ptr = (SQLPOINTER)ecalloc(1, in_length+1);
				if ( out_ptr == NULL ) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for XML Data");
					RETURN_FALSE;
				}
				rc = _php_db2_get_data(stmt_res, col_num+1, SQL_C_BINARY, (SQLPOINTER)out_ptr, in_length, &out_length TSRMLS_CC);
				if (rc == SQL_ERROR) {
					RETURN_FALSE;
				}
				RETVAL_STRINGL((char*)out_ptr,out_length, 0);
#endif /* not  PASE */
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
	SQLINTEGER out_length, loc_length, tmp_length;
	unsigned char *out_ptr;
#ifdef PASE /* i5/OS V6R1 incompatible change */
	if (is_i5os_classic) {
		lob_bind_type= SQL_C_BINARY;
	} else {
		lob_bind_type= SQL_C_BINARY_V6;
	}
#endif /* PASE */

	if (zend_parse_parameters(argc TSRMLS_CC, "r|l", &stmt, &row_number) == FAILURE) {
		return;
	}

	if ( stmt ) {
		ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);
	}

	_php_db2_init_error_info(stmt_res);

	/* get column header info*/
	if ( stmt_res->column_info == NULL ) {
		if (_php_db2_get_result_set_info(stmt_res TSRMLS_CC) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column information cannot be retrieved");
			RETURN_FALSE;
		}
	}
	/* bind the data */
	if ( stmt_res->row_data == NULL ) {
		rc = _php_db2_bind_column_helper(stmt_res TSRMLS_CC);
		if ( rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column binding cannot be done");
			RETURN_FALSE;
		}
	}
#ifdef PASE /* i5/OS problem with SQL_FETCH out_length (temporary until fixed) */
	for (i = 0; i<stmt_res->num_columns; i++) {
		stmt_res->row_data[i].out_length = 0;
	}
#endif /*PASE*/
	/* check if row_number is present */
	if (argc == 2 && row_number > 0) {
#ifndef PASE /* i5/OS problem with SQL_FETCH_ABSOLUTE (temporary until fixed) */
		rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_ABSOLUTE, row_number);
#else /*PASE */
		rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_FIRST, row_number);
		if (row_number>1 && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO))
			rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_RELATIVE, row_number-1);
#endif /*PASE*/
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (argc == 2 && row_number < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Requested row number must be a positive value");
		RETURN_FALSE;
	} else {
		/*row_number is NULL or 0; just fetch next row*/
		rc = SQLFetch((SQLHSTMT)stmt_res->hstmt);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	}

	if (rc == SQL_NO_DATA_FOUND) {
		RETURN_FALSE;
	} else if ( rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Fetch Failure");
		RETURN_FALSE;
	}
	/* copy the data over return_value */
	array_init(return_value);
	for (i=0; i<stmt_res->num_columns; i++) {
		column_type = stmt_res->column_info[i].type;
		row_data = &stmt_res->row_data[i].data;
		out_length = stmt_res->row_data[i].out_length;
		loc_length = stmt_res->column_info[i].loc_ind;

		switch(stmt_res->s_case_mode) {
			case DB2_CASE_LOWER:
				stmt_res->column_info[i].name = (SQLCHAR *)php_strtolower((char *)stmt_res->column_info[i].name, strlen((char *)stmt_res->column_info[i].name));
				break;
			case DB2_CASE_UPPER:
				stmt_res->column_info[i].name = (SQLCHAR *)php_strtoupper((char *)stmt_res->column_info[i].name, strlen((char *)stmt_res->column_info[i].name));
				break;
			case DB2_CASE_NATURAL:
			default:
				break;
		}

		if (out_length == SQL_NULL_DATA || loc_length == SQL_NULL_DATA) {
			if ( op & DB2_FETCH_ASSOC ) {
				add_assoc_null(return_value, (char *)stmt_res->column_info[i].name);
			}
			if ( op & DB2_FETCH_INDEX ) {
				add_index_null(return_value, i);
			}
		} else {
			switch(column_type) {
				case SQL_CHAR:
				case SQL_VARCHAR:
#ifdef PASE
				case SQL_UTF8_CHAR:
#endif
				case SQL_WCHAR:
				case SQL_WVARCHAR:
				case SQL_GRAPHIC:
				case SQL_VARGRAPHIC:
#ifndef PASE /* i5/OS SQL_DBCLOB */
				case SQL_DBCLOB:
#endif /* not PASE */
#ifndef PASE /* i5/OS SQL_LONGVARCHAR is SQL_VARCHAR */
				case SQL_LONGVARCHAR:
				case SQL_WLONGVARCHAR:
				case SQL_LONGVARGRAPHIC:
#endif /* not PASE */
				case SQL_TYPE_DATE:
				case SQL_TYPE_TIME:
				case SQL_TYPE_TIMESTAMP:
				case SQL_DATETIME:
				case SQL_BIGINT:
				case SQL_DECIMAL:
				case SQL_NUMERIC:
				case SQL_DECFLOAT:

					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name,
							(char *)row_data->str_val, strlen((char *)row_data->str_val), 1);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_stringl(return_value, i, (char *)row_data->str_val,
							strlen((char *)row_data->str_val), 1);
					}
					break;
				case SQL_SMALLINT:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_long(return_value, (char *)stmt_res->column_info[i].name, row_data->s_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_long(return_value, i, row_data->s_val);
					}
					break;
				case SQL_INTEGER:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_long(return_value, (char *)stmt_res->column_info[i].name,
							row_data->i_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_long(return_value, i, row_data->i_val);
					}
					break;

				case SQL_REAL:
#ifndef PASE /* LUW need this? */
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_double(return_value, (char *)stmt_res->column_info[i].name, row_data->r_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_double(return_value, i, row_data->r_val);
					}
					break;
#endif
				case SQL_FLOAT:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_double(return_value, (char *)stmt_res->column_info[i].name, row_data->f_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_double(return_value, i, row_data->f_val);
					}
					break;

				case SQL_DOUBLE:
					if ( op & DB2_FETCH_ASSOC ) {
						add_assoc_double(return_value, (char *)stmt_res->column_info[i].name, row_data->d_val);
					}
					if ( op & DB2_FETCH_INDEX ) {
						add_index_double(return_value, i, row_data->d_val);
					}
					break;

#ifdef PASE /* i5/OS incompatible v6r1 change */
				case SQL_VARBINARY_V6:
				case SQL_BINARY_V6:
#endif /* PASE */
				case SQL_BINARY:
#ifndef PASE /* i5/OS SQL_LONGVARBINARY is SQL_VARBINARY */
				case SQL_LONGVARBINARY:
#endif /* PASE */
				case SQL_VARBINARY:
					if ( stmt_res->s_bin_mode == DB2_PASSTHRU ) {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name, "", 0, 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, "", 0, 1);
						}
					} else {
#ifdef PASE /* i5/OS BINARY is strlen incompatible (may work for LUW) */
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name,
								(char *)row_data->str_val, out_length, 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, (char *)row_data->str_val,
								out_length, 1);
						}
#else
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name,
								(char *)row_data->str_val, strlen((char *)row_data->str_val), 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, (char *)row_data->str_val,
								strlen((char *)row_data->str_val), 1);
						}
#endif /* PASE */
					}
					break;
				case SQL_BLOB:
					out_ptr = NULL;
					rc=_php_db2_get_length(stmt_res, i+1, &tmp_length TSRMLS_CC);
					if ( rc == SQL_ERROR ) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Determine LOB Size");
						RETURN_FALSE;
					}

					if (tmp_length == SQL_NULL_DATA) {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_null(return_value, (char *)stmt_res->column_info[i].name);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_null(return_value, i);
						}
					} else {
						switch (stmt_res->s_bin_mode) {
							case DB2_PASSTHRU:
								if ( op & DB2_FETCH_ASSOC ) {
									add_assoc_null(return_value, (char *)stmt_res->column_info[i].name);
								}
								if ( op & DB2_FETCH_INDEX ) {
									add_index_null(return_value, i);
								}
								break;
							case DB2_CONVERT:
#ifndef PASE /* i5/OS not supported */
								tmp_length = 2*tmp_length + 1;
								lob_bind_type = SQL_C_CHAR;
								/* fall-through */
#endif /* not PASE */
							case DB2_BINARY:
								out_ptr = (SQLPOINTER)ecalloc(1, tmp_length);

								if ( out_ptr == NULL ) {
									php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
									RETURN_FALSE;
								}
								rc = _php_db2_get_data2(stmt_res, i+1, lob_bind_type, (char *)out_ptr, tmp_length, tmp_length, &out_length TSRMLS_CC);
								if (rc == SQL_ERROR) {
									RETURN_FALSE;
								}

								if ( op & DB2_FETCH_ASSOC ) {
									add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name, (char *)out_ptr, out_length, 1);
								}
								if ( op & DB2_FETCH_INDEX ) {
									add_index_stringl(return_value, i, (char *)out_ptr, out_length, DB2_FETCH_BOTH & op);
								}

								efree(out_ptr);
								break;
							default:
								break;
						}
					}
					break;

				case SQL_XML:
					out_ptr = NULL;
#ifdef PASE /* i5/OS V6R1 incompatible change */
					if (is_i5os_classic){
						rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY, NULL, 0, (SQLINTEGER *)&tmp_length TSRMLS_CC);
					} else {
						rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY_V6, NULL, 0, (SQLINTEGER *)&tmp_length TSRMLS_CC);
					}
#else
                    rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY, NULL, 0, (SQLINTEGER *)&tmp_length TSRMLS_CC);
#endif /* PASE */
					if ( rc == SQL_ERROR ) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Determine XML Size");
						RETURN_FALSE;
					}

					if (tmp_length == SQL_NULL_DATA) {
						if ( op & DB2_FETCH_ASSOC ) {
								add_assoc_null(return_value, (char *)stmt_res->column_info[i].name);
							}
							if ( op & DB2_FETCH_INDEX ) {
								add_index_null(return_value, i);
							}
						} else {
							out_ptr = (SQLPOINTER)ecalloc(1, tmp_length+1);

							if ( out_ptr == NULL ) {
								php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for XML Data");
								RETURN_FALSE;
							}
#ifdef PASE /* i5/OS V6R1 incompatible change */
							if (is_i5os_classic){
								rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY, out_ptr, tmp_length, &out_length TSRMLS_CC);
							} else {
								rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY_V6, out_ptr, tmp_length, &out_length TSRMLS_CC);
							}
#else
							rc = _php_db2_get_data(stmt_res, i+1, SQL_C_BINARY, out_ptr, tmp_length, &out_length TSRMLS_CC);
#endif /* PASE */
							if (rc == SQL_ERROR) {
								efree(out_ptr);
								RETURN_FALSE;
							}

							if ( op & DB2_FETCH_ASSOC ) {
								add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name, (char *)out_ptr, out_length, 1);
							}
							if ( op & DB2_FETCH_INDEX ) {
								add_index_stringl(return_value, i, (char *)out_ptr, out_length, DB2_FETCH_BOTH & op);
							}

							efree(out_ptr);
						}
						break;

#ifdef PASE /* i5/OS SQL_DBCLOB */
				case SQL_DBCLOB:
#endif /* not PASE */
				case SQL_CLOB:
					out_ptr = NULL;
					rc = _php_db2_get_length(stmt_res, i+1, &tmp_length TSRMLS_CC);
					if ( rc == SQL_ERROR ) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Determine LOB Size");
						RETURN_FALSE;
					}

					if (tmp_length == SQL_NULL_DATA) {
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_null(return_value, (char *)stmt_res->column_info[i].name);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_null(return_value, i);
						}
					} else {
						out_ptr = (SQLPOINTER)ecalloc(1, tmp_length+1);
						if ( out_ptr == NULL ) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot Allocate Memory for LOB Data");
							RETURN_FALSE;
						}
						rc = _php_db2_get_data2(stmt_res, i+1, SQL_C_CHAR, out_ptr, tmp_length, tmp_length+1, &out_length TSRMLS_CC);
						if (rc == SQL_ERROR) {
							efree(out_ptr);
							RETURN_FALSE;
						}

#ifdef PASE /* i5/OS seem to get +1 back from SQLGetSubString  */
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, 
							(char*)stmt_res->column_info[i].name, 
							(char *)out_ptr, out_length-1, 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, 
							(char*)out_ptr, out_length-1, 
							DB2_FETCH_BOTH & op);
						}
#else
						if ( op & DB2_FETCH_ASSOC ) {
							add_assoc_stringl(return_value, (char *)stmt_res->column_info[i].name, (char *)out_ptr, out_length, 1);
						}
						if ( op & DB2_FETCH_INDEX ) {
							add_index_stringl(return_value, i, (char *)out_ptr, out_length, DB2_FETCH_BOTH & op);
						}
#endif /* PASE */
						efree(out_ptr);
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

    /* get column header info*/
	if ( stmt_res->column_info == NULL ) {
		if (_php_db2_get_result_set_info(stmt_res TSRMLS_CC)<0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column information cannot be retrieved");
			RETURN_FALSE;
		}
	}

	/*check if row_number is present*/
	if (argc == 2 && row_number > 0) {
#ifndef PASE /* i5/OS problem with SQL_FETCH_ABSOLUTE */
		rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_ABSOLUTE, row_number);
#else /*PASE */
		rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_FIRST, row_number);
		if (row_number>1 && (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO))
			rc = SQLFetchScroll((SQLHSTMT)stmt_res->hstmt, SQL_FETCH_RELATIVE, row_number-1);
#endif /*PASE*/
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
	} else if (argc == 2 && row_number < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Requested row number must be a positive value");
		RETURN_FALSE;
	} else {
		/*row_number is NULL or 0; just fetch next row*/
		rc = SQLFetch((SQLHSTMT)stmt_res->hstmt);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHSTMT)stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}
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

/* {{{ proto bool db2_set_option(resource resc, array options, int type)
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
#ifdef PASE /* always change the libl if requested by options array */
			_php_db2_i5cmd_helper(conn_res);
#endif /* PASE */
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

/* {{{ proto object db2_server_info(resource connection)
Returns an object with properties that describe the DB2 database server */
PHP_FUNCTION(db2_server_info)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc = 0;
	SQLCHAR buffer11[11];
	SQLCHAR buffer255[255];
	SQLCHAR buffer2k[2048];
	SQLSMALLINT bufferint16;
	SQLUINTEGER bufferint32;
	SQLINTEGER bitmask;

	object_init(return_value);

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		/* DBMS_NAME */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DBMS_NAME, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "DBMS_NAME", (char *)buffer255, strlen((char *)buffer255), 1);
		}

		/* DBMS_VER */
		memset(buffer11, 0, sizeof(buffer11));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DBMS_VER, (SQLPOINTER)buffer11, sizeof(buffer11), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "DBMS_VER", (char *)buffer11, strlen((char *)buffer11), 1);
		}

#ifndef PASE    /* i5/OS DB_CODEPAGE handled natively */
		/* DB_CODEPAGE */
		bufferint32 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_DATABASE_CODEPAGE, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "DB_CODEPAGE", bufferint32);
		}
#endif /* PASE */

		/* DB_NAME */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DATABASE_NAME, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
#ifndef PASE    /* i5/OS DB_NAME not available V5R3 (skip) */
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
#endif /* not PASE */
		} else {
			add_property_stringl(return_value, "DB_NAME", (char *)buffer255, strlen((char *)buffer255), 1);
		}

#ifndef PASE    /* i5/OS INST_NAME handled natively */
		/* INST_NAME */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_SERVER_NAME, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "INST_NAME", (char *)buffer255, strlen((char *)buffer255), 1);
		}
#endif /* PASE */

#ifndef PASE    /* i5/OS SPECIAL_CHARS handled natively */
		/* SPECIAL_CHARS */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_SPECIAL_CHARACTERS, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "SPECIAL_CHARS", (char *)buffer255, strlen((char *)buffer255), 1);
		}
#endif /* PASE */

		/* KEYWORDS */
		memset(buffer2k, 0, sizeof(buffer2k));
		rc = SQLGetInfo(conn_res->hdbc, SQL_KEYWORDS, (SQLPOINTER)buffer2k, sizeof(buffer2k), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			char *keyword, *last;
			int key = 0;
			zval *karray;

			MAKE_STD_ZVAL(karray);

			array_init(karray);

			keyword = php_strtok_r((char *)buffer2k, ",", &last);
			while (keyword) {
				add_index_stringl(karray, key++, keyword, strlen(keyword), 1);
				keyword = php_strtok_r(NULL, ",", &last);
			}

			add_property_zval(return_value, "KEYWORDS", karray);

			zval_ptr_dtor(&karray);
		}

		/* DFT_ISOLATION */
		bitmask = 0;
		memset(buffer11, 0, sizeof(buffer11));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DEFAULT_TXN_ISOLATION, &bitmask, sizeof(bitmask), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			if( bitmask & SQL_TXN_READ_UNCOMMITTED ) {
				strcpy((char *)buffer11, "UR");
			}
			if( bitmask & SQL_TXN_READ_COMMITTED ) {
				strcpy((char *)buffer11, "CS");
			}
			if( bitmask & SQL_TXN_REPEATABLE_READ ) {
				strcpy((char *)buffer11, "RS");
			}
			if( bitmask & SQL_TXN_SERIALIZABLE ) {
				strcpy((char *)buffer11, "RR");
			}
			if( bitmask & SQL_TXN_NOCOMMIT ) {
				strcpy((char *)buffer11, "NC");
			}

			add_property_stringl(return_value, "DFT_ISOLATION", (char *)buffer11, strlen((char *)buffer11), 1);
		}

		/* ISOLATION_OPTION */
		bitmask = 0;
		memset(buffer11, 0, sizeof(buffer11));
#ifdef PASE    /* i5/OS ISOLATION_OPTION */
		rc = SQLGetInfo(conn_res->hdbc, SQL_DEFAULT_TXN_ISOLATION, &bitmask, sizeof(bitmask), NULL);
#else
		rc = SQLGetInfo(conn_res->hdbc, SQL_TXN_ISOLATION_OPTION, &bitmask, sizeof(bitmask), NULL);
#endif /* PASE */
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			int key = 0;
			zval *array;

			MAKE_STD_ZVAL(array);

			array_init(array);

			if( bitmask & SQL_TXN_READ_UNCOMMITTED ) {
				add_index_stringl(array, key++, "UR", 2, 1);
			}
			if( bitmask & SQL_TXN_READ_COMMITTED ) {
				add_index_stringl(array, key++, "CS", 2, 1);
			}
			if( bitmask & SQL_TXN_REPEATABLE_READ ) {
				add_index_stringl(array, key++, "RS", 2, 1);
			}
			if( bitmask & SQL_TXN_SERIALIZABLE ) {
				add_index_stringl(array, key++, "RR", 2, 1);
			}
			if( bitmask & SQL_TXN_NOCOMMIT ) {
				add_index_stringl(array, key++, "NC", 2, 1);
			}

			add_property_zval(return_value, "ISOLATION_OPTION", array);

			zval_ptr_dtor(&array);
		}

		/* SQL_CONFORMANCE */
		bufferint32 = 0;
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_ODBC_SQL_CONFORMANCE, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			switch (bufferint32) {
				case SQL_SC_SQL92_ENTRY:
					strcpy((char *)buffer255, "ENTRY");
					break;
				case SQL_SC_FIPS127_2_TRANSITIONAL:
					strcpy((char *)buffer255, "FIPS127");
					break;
				case SQL_SC_SQL92_FULL:
					strcpy((char *)buffer255, "FULL");
					break;
				case SQL_SC_SQL92_INTERMEDIATE:
					strcpy((char *)buffer255, "INTERMEDIATE");
					break;
				default:
					break;
			}
			add_property_stringl(return_value, "SQL_CONFORMANCE", (char *)buffer255, strlen((char *)buffer255), 1);
		}

		/* PROCEDURES */
		memset(buffer11, 0, sizeof(buffer11));
		rc = SQLGetInfo(conn_res->hdbc, SQL_PROCEDURES, (SQLPOINTER)buffer11, sizeof(buffer11), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			if( strcmp((char *)buffer11, "Y") == 0 ) {
				add_property_bool(return_value, "PROCEDURES", 1);
			} else {
				add_property_bool(return_value, "PROCEDURES", 0);
			}
		}

		/* IDENTIFIER_QUOTE_CHAR */
		memset(buffer11, 0, sizeof(buffer11));
		rc = SQLGetInfo(conn_res->hdbc, SQL_IDENTIFIER_QUOTE_CHAR, (SQLPOINTER)buffer11, sizeof(buffer11), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "IDENTIFIER_QUOTE_CHAR", (char *)buffer11, strlen((char *)buffer11), 1);
		}

		/* LIKE_ESCAPE_CLAUSE */
		memset(buffer11, 0, sizeof(buffer11));
		rc = SQLGetInfo(conn_res->hdbc, SQL_LIKE_ESCAPE_CLAUSE, (SQLPOINTER)buffer11, sizeof(buffer11), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			if( strcmp((char *)buffer11, "Y") == 0 ) {
				add_property_bool(return_value, "LIKE_ESCAPE_CLAUSE", 1);
			} else {
				add_property_bool(return_value, "LIKE_ESCAPE_CLAUSE", 0);
			}
		}

		/* MAX_COL_NAME_LEN */
		bufferint16 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_COLUMN_NAME_LEN, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_COL_NAME_LEN", bufferint16);
		}

		/* MAX_ROW_SIZE */
		bufferint32 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_ROW_SIZE, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_ROW_SIZE", bufferint32);
		}

#ifndef PASE    /* i5/OS MAX_IDENTIFIER_LEN handled natively */
		/* MAX_IDENTIFIER_LEN */
		bufferint16 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_IDENTIFIER_LEN, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_IDENTIFIER_LEN", bufferint16);
		}
#endif /* PASE */

#ifndef PASE    /* i5/OS MAX_INDEX_SIZE handled natively */
		/* MAX_INDEX_SIZE */
		bufferint32 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_INDEX_SIZE, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_INDEX_SIZE", bufferint32);
		}
#endif /* PASE */

#ifndef PASE    /* i5/OS MAX_PROC_NAME_LEN handled natively */
		/* MAX_PROC_NAME_LEN */
		bufferint16 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_PROCEDURE_NAME_LEN, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_PROC_NAME_LEN", bufferint16);
		}
#endif /* PASE */

		/* MAX_SCHEMA_NAME_LEN */
		bufferint16 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_SCHEMA_NAME_LEN, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_SCHEMA_NAME_LEN", bufferint16);
		}

		/* MAX_STATEMENT_LEN */
		bufferint32 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_STATEMENT_LEN, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_STATEMENT_LEN", bufferint32);
		}

		/* MAX_TABLE_NAME_LEN */
		bufferint16 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_MAX_TABLE_NAME_LEN, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "MAX_TABLE_NAME_LEN", bufferint16);
		}

		/* NON_NULLABLE_COLUMNS */
		bufferint16 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_NON_NULLABLE_COLUMNS, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			int rv;
			switch (bufferint16) {
				case SQL_NNC_NON_NULL:
					rv = 1;
					break;
				case SQL_NNC_NULL:
					rv = 0;
					break;
				default:
					break;
			}
			add_property_bool(return_value, "NON_NULLABLE_COLUMNS", rv);
		}

	return;
	}
}
/* }}} */

/* {{{ proto object db2_client_info(resource connection)
Returns an object with properties that describe the DB2 database client */
PHP_FUNCTION(db2_client_info)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc = 0;
	SQLCHAR buffer255[255];
	SQLSMALLINT bufferint16;
	SQLUINTEGER bufferint32;

	object_init(return_value);

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		/* DRIVER_NAME */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DRIVER_NAME, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "DRIVER_NAME", (char *)buffer255, strlen((char *)buffer255), 1);
		}

		/* DRIVER_VER */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DRIVER_VER, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "DRIVER_VER", (char *)buffer255, strlen((char *)buffer255), 1);
		}

		/* DATA_SOURCE_NAME */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DATA_SOURCE_NAME, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "DATA_SOURCE_NAME", (char *)buffer255, strlen((char *)buffer255), 1);
		}

		/* DRIVER_ODBC_VER */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_DRIVER_ODBC_VER, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "DRIVER_ODBC_VER", (char *)buffer255, strlen((char *)buffer255), 1);
		}

#ifndef PASE    /* i5/OS ODBC_VER handled natively */
		/* ODBC_VER */
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_ODBC_VER, (SQLPOINTER)buffer255, sizeof(buffer255), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_stringl(return_value, "ODBC_VER", (char *)buffer255, strlen((char *)buffer255), 1);
		}
#endif /* PASE */

		/* ODBC_SQL_CONFORMANCE */
		bufferint16 = 0;
		memset(buffer255, 0, sizeof(buffer255));
		rc = SQLGetInfo(conn_res->hdbc, SQL_ODBC_SQL_CONFORMANCE, &bufferint16, sizeof(bufferint16), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			switch (bufferint16) {
				case SQL_OSC_MINIMUM:
					strcpy((char *)buffer255, "MINIMUM");
					break;
				case SQL_OSC_CORE:
					strcpy((char *)buffer255, "CORE");
					break;
				case SQL_OSC_EXTENDED:
					strcpy((char *)buffer255, "EXTENDED");
					break;
				default:
					break;
			}
			add_property_stringl(return_value, "ODBC_SQL_CONFORMANCE", (char *)buffer255, strlen((char *)buffer255), 1);
		}

#ifndef PASE    /* i5/OS APPL_CODEPAGE handled natively */
		/* APPL_CODEPAGE */
		bufferint32 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_APPLICATION_CODEPAGE, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "APPL_CODEPAGE", bufferint32);
		}
#endif /* PASE */

#ifndef PASE    /* i5/OS CONN_CODEPAGE handled natively */
		/* CONN_CODEPAGE */
		bufferint32 = 0;
		rc = SQLGetInfo(conn_res->hdbc, SQL_CONNECT_CODEPAGE, &bufferint32, sizeof(bufferint32), NULL);

		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors(conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
			RETURN_FALSE;
			return;
		} else {
			add_property_long(return_value, "CONN_CODEPAGE", bufferint32);
		}
#endif /* PASE */

	return;
	}
}
/* }}} */

/* {{{ proto string db2_escape_string(string unescaped_string)		 
   Escapes a string for use in a SQL statement */		 
PHP_FUNCTION(db2_escape_string)	 
{	 
	int argc = ZEND_NUM_ARGS();	 
	char *str, *new_str;	 
	char *source, *target;		 
	char *end;		 
	int new_length;	 
	int length;	 

	if (zend_parse_parameters(argc TSRMLS_CC, "s", &str, &length) == FAILURE) {	 
		return;		 
	}	 

	if (!str) {	 
		RETURN_EMPTY_STRING();	 
	}	 

	/* allocate twice the source length first (worst case) */	 
	new_str = (char*)malloc(((length*2)+1)*sizeof(char));	 

	source = str;	 
	end = source + length;		 
	target = new_str;	 

	while (source < end) {		 
		switch( *source ) {		 	 
			case '\'':
				*target++ = '\'';		 
				*target++ = '\'';		 
				break;	 	 
			default:		 
				*target++ = *source;	 
				break;	 
		}	 
		source++;	 
	}	 

	/* terminate the string and calculate the real length */	 
	*target = 0;	 
	new_length = target - new_str;		 


	/* reallocate to the real length */	 
	new_str = (char *)realloc(new_str, new_length + 1);	 

	RETURN_STRINGL(new_str, new_length, 1)		 
}	 
/* }}} */

/* {{{ proto resource db2_lob_read(resource stmt, column_number, lob_length)
Returns part of a lob, specifically lob_length of the column_number specified */
PHP_FUNCTION(db2_lob_read)
{
	int argc = ZEND_NUM_ARGS();
	int stmt_id = -1;
	zval *stmt = NULL;
	stmt_handle *stmt_res;
	int rc, i = 0;
	SQLINTEGER out_length, length=BUFSIZ, colnum = 1;
	void *out_ptr = NULL;

	/* Parse out the parameters */
	if (zend_parse_parameters(argc TSRMLS_CC, "rll", &stmt, &colnum, &length) == FAILURE) {
		return;
	}

	/* Ensure the statement was passed in */
	if (!stmt) {
		return;
	}

	/* Get the statement handle */
	ZEND_FETCH_RESOURCE(stmt_res, stmt_handle*, &stmt, stmt_id, "Statement Resource", le_stmt_struct);

	/* Get the data in the amount of length */
#ifdef PASE /* PASE */
	out_ptr = (SQLPOINTER)ecalloc(1, length+1);
	((char *)out_ptr)[length]='\0';
#else
	out_ptr = (SQLPOINTER)ecalloc(1, ++length);
#endif /* PASE */
	rc = SQLGetData((SQLHSTMT)stmt_res->hstmt, colnum, SQL_C_CHAR, (SQLPOINTER)out_ptr, length, &out_length);
	if ( rc == SQL_NO_DATA_FOUND ) {
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		efree(out_ptr);
		RETURN_FALSE;
	}
	if ( rc == SQL_ERROR ) {
		_php_db2_check_sql_errors(stmt_res->hstmt, SQL_HANDLE_STMT, rc, 1, NULL, -1, 1 TSRMLS_CC);
		efree(out_ptr);
		RETURN_FALSE;
	}

	RETVAL_STRING((char*)out_ptr, 1);
	efree(out_ptr);
}
/* }}} */

/* {{{ proto resource db2_get_option(resource connection, string option)
Returns the current setting of the connection attribute provided */
PHP_FUNCTION(db2_get_option)
{
	int argc = ZEND_NUM_ARGS();
	zval *connection = NULL;
	char *option = NULL;
	int option_len = 0;
	conn_handle *conn_res;
	SQLCHAR *value = NULL;
	int connection_id = -1;
	int rc;
	int val = 0;

	/* Parse out the parameters */
	if (zend_parse_parameters(argc TSRMLS_CC, "rs", &connection, &option, &option_len) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);

		if (!conn_res->handle_active) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection is not active");
			RETURN_FALSE;
		}

#ifdef PASE  /* i5/OS v6r1 support only */
		if (is_i5os_classic) {
		  php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect option string passed in");
		  RETURN_FALSE;
		}
		else {
#endif /* PASE */
		if (option) {
			if (!STRCASECMP(option, "userid")) {
				value = ecalloc(1, USERID_LEN + 1);
				rc = SQLGetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_INFO_USERID, (SQLPOINTER)value, USERID_LEN, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
			} else if (!STRCASECMP(option, "acctstr")) {
				value = ecalloc(1, ACCTSTR_LEN + 1);
				rc = SQLGetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_INFO_ACCTSTR, (SQLPOINTER)value, ACCTSTR_LEN, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
			} else if (!STRCASECMP(option, "applname")) {
				value = ecalloc(1, APPLNAME_LEN + 1);
				rc = SQLGetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_INFO_APPLNAME, (SQLPOINTER)value, APPLNAME_LEN, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
			} else if (!STRCASECMP(option, "wrkstnname")) {
				value = ecalloc(1, WRKSTNNAME_LEN + 1);
				rc = SQLGetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_INFO_WRKSTNNAME, (SQLPOINTER)value, WRKSTNNAME_LEN, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
#ifndef PASE  /* i5/OS no support yet */
			} else if(!STRCASECMP(option, "trustedcontext")) {
				rc = SQLGetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_USE_TRUSTED_CONTEXT, (SQLPOINTER)&val, 0, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
				if(val == SQL_TRUE) {
					RETURN_TRUE;
				} else {
					RETURN_FALSE;
				}
			} else if (!STRCASECMP(option, "trusted_user")) {
				value = ecalloc(1, USERID_LEN + 1);
				rc = SQLGetConnectAttr((SQLHDBC)conn_res->hdbc, SQL_ATTR_TRUSTED_CONTEXT_USERID, (SQLPOINTER)value, USERID_LEN, NULL);
				if ( rc == SQL_ERROR ) {
					_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
					RETURN_FALSE;
				}
#endif /* not PASE */
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incorrect option string passed in");
				RETURN_FALSE;
			}
			RETURN_STRING(value, 0);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Supplied parameter is invalid");
			RETURN_FALSE;
		}
#ifdef PASE  /* i5/OS v6r1 support only */
		}
#endif /* PASE */
	}
}
/* }}} */

/* {{{ proto resource db2_last_insert_id(resource connection)
Returns the last insert id as a string. */
PHP_FUNCTION(db2_last_insert_id)
{
	int argc = ZEND_NUM_ARGS();
	int connection_id = -1;
	zval *connection = NULL;
	conn_handle *conn_res;
	int rc;
	char *last_id = emalloc( MAX_IDENTITY_DIGITS );
	char *sql;
	SQLHANDLE hstmt;
#ifdef PASE /* SQLINTEGER vs. SQLUINTEGER */
	SQLINTEGER out_length;
#else
	SQLUINTEGER out_length;
#endif

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &connection) == FAILURE) {
		return;
	}

	if (connection) {
		ZEND_FETCH_RESOURCE2(conn_res, conn_handle*, &connection, connection_id,
			"Connection Resource", le_conn_struct, le_pconn_struct);
		
		/* get a new statement handle */
		strcpy( last_id, "" );
		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn_res->hdbc, &hstmt);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}

		/* Selecting last insert ID from current connection resource. */
		sql = "SELECT IDENTITY_VAL_LOCAL() FROM SYSIBM.SYSDUMMY1";
		rc = SQLExecDirect(hstmt, (SQLCHAR *) sql, strlen(sql));
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}

		/* Binding and fetching last insert ID. */
		rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, last_id, MAX_IDENTITY_DIGITS, &out_length);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}

		rc = SQLFetch(hstmt);
		if ( rc == SQL_ERROR ) {
			_php_db2_check_sql_errors((SQLHDBC)conn_res->hdbc, SQL_HANDLE_DBC, rc, 1, NULL, -1, 1 TSRMLS_CC);
		}

		/* Free allocated statement handle. */
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

		/* Returning last insert ID (if any), or otherwise NULL */
		if(last_id != "") {
			RETURN_STRING(last_id, 0);
		} else {
			RETURN_NULL();
		}
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
