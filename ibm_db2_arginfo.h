/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3e863612a7a676e042e024f6778b791bd086abe5 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_connect, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, database, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, username, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, password, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_commit, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, connection)
ZEND_END_ARG_INFO()

#define arginfo_db2_pconnect arginfo_db2_connect

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_autocommit, 0, 1, MAY_BE_LONG|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_bind_param, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, parameter_number, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, variable_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, parameter_type, IS_LONG, 0, "DB2_PARAM_IN")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data_type, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, precision, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_db2_close arginfo_db2_commit

#if defined(PASE)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_pclose, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, connection)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_column_privileges, 0, 0, 1)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, qualifier, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table_name, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, column_name, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_db2_columnprivileges arginfo_db2_column_privileges

#define arginfo_db2_columns arginfo_db2_column_privileges

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_foreign_keys, 0, 0, 4)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, qualifier, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, schema, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, table_name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_db2_foreignkeys arginfo_db2_foreign_keys

#define arginfo_db2_primary_keys arginfo_db2_foreign_keys

#define arginfo_db2_primarykeys arginfo_db2_foreign_keys

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_procedure_columns, 0, 0, 5)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, qualifier, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, procedure, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, parameter, IS_STRING, 1)
ZEND_END_ARG_INFO()

#define arginfo_db2_procedurecolumns arginfo_db2_procedure_columns

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_procedures, 0, 0, 4)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, qualifier, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, procedure, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_special_columns, 0, 0, 5)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, qualifier, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, table_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, scope, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_db2_specialcolumns arginfo_db2_special_columns

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_statistics, 0, 0, 5)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, qualifier, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, schema, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, table_name, IS_STRING, 0)
	ZEND_ARG_TYPE_MASK(0, unique, MAY_BE_BOOL|MAY_BE_LONG, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_table_privileges, 0, 0, 1)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, qualifier, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table_name, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_db2_tableprivileges arginfo_db2_table_privileges

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_tables, 0, 0, 1)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, qualifier, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table_name, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table_type, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_exec, 0, 0, 2)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_TYPE_INFO(0, statement, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

#define arginfo_db2_prepare arginfo_db2_exec

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_execute, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

#if !defined(PASE)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_execute_many, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_stmt_errormsg, 0, 0, IS_STRING, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, stmt, "null")
ZEND_END_ARG_INFO()

#define arginfo_db2_stmt_error arginfo_db2_stmt_errormsg

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_conn_errormsg, 0, 0, IS_STRING, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, connection, "null")
ZEND_END_ARG_INFO()

#define arginfo_db2_conn_error arginfo_db2_conn_errormsg

ZEND_BEGIN_ARG_INFO_EX(arginfo_db2_next_result, 0, 0, 1)
	ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_num_fields, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

#define arginfo_db2_num_rows arginfo_db2_num_fields

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_field_name, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_MASK(0, column, MAY_BE_LONG|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_field_display_size, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_MASK(0, column, MAY_BE_LONG|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

#define arginfo_db2_field_num arginfo_db2_field_display_size

#define arginfo_db2_field_precision arginfo_db2_field_display_size

#define arginfo_db2_field_scale arginfo_db2_field_display_size

#define arginfo_db2_field_type arginfo_db2_field_name

#define arginfo_db2_field_width arginfo_db2_field_display_size

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_cursor_type, 0, 1, IS_LONG, 0)
	ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

#define arginfo_db2_rollback arginfo_db2_commit

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_free_stmt, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_result, 0, 2, IS_MIXED, 1)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_MASK(0, column, MAY_BE_LONG|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_fetch_row, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row_number, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_fetch_assoc, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row_number, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_db2_fetch_array arginfo_db2_fetch_assoc

#define arginfo_db2_fetch_both arginfo_db2_fetch_assoc

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_fetch_object, 0, 1, MAY_BE_OBJECT|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row_number, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_db2_free_result arginfo_db2_free_stmt

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_set_option, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_db2_setoption arginfo_db2_set_option

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_client_info, 0, 1, MAY_BE_OBJECT|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, connection)
ZEND_END_ARG_INFO()

#define arginfo_db2_server_info arginfo_db2_client_info

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_escape_string, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, string_literal, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_lob_read, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, stmt)
	ZEND_ARG_TYPE_INFO(0, colnum, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_db2_get_option, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_db2_getoption arginfo_db2_get_option

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_db2_last_insert_id, 0, 1, IS_STRING, 0)
	ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(db2_connect);
ZEND_FUNCTION(db2_commit);
ZEND_FUNCTION(db2_pconnect);
ZEND_FUNCTION(db2_autocommit);
ZEND_FUNCTION(db2_bind_param);
ZEND_FUNCTION(db2_close);
#if defined(PASE)
ZEND_FUNCTION(db2_pclose);
#endif
ZEND_FUNCTION(db2_column_privileges);
ZEND_FUNCTION(db2_columns);
ZEND_FUNCTION(db2_foreign_keys);
ZEND_FUNCTION(db2_primary_keys);
ZEND_FUNCTION(db2_procedure_columns);
ZEND_FUNCTION(db2_procedures);
ZEND_FUNCTION(db2_special_columns);
ZEND_FUNCTION(db2_statistics);
ZEND_FUNCTION(db2_table_privileges);
ZEND_FUNCTION(db2_tables);
ZEND_FUNCTION(db2_exec);
ZEND_FUNCTION(db2_prepare);
ZEND_FUNCTION(db2_execute);
#if !defined(PASE)
ZEND_FUNCTION(db2_execute_many);
#endif
ZEND_FUNCTION(db2_stmt_errormsg);
ZEND_FUNCTION(db2_stmt_error);
ZEND_FUNCTION(db2_conn_errormsg);
ZEND_FUNCTION(db2_conn_error);
ZEND_FUNCTION(db2_next_result);
ZEND_FUNCTION(db2_num_fields);
ZEND_FUNCTION(db2_num_rows);
ZEND_FUNCTION(db2_field_name);
ZEND_FUNCTION(db2_field_display_size);
ZEND_FUNCTION(db2_field_num);
ZEND_FUNCTION(db2_field_precision);
ZEND_FUNCTION(db2_field_scale);
ZEND_FUNCTION(db2_field_type);
ZEND_FUNCTION(db2_field_width);
ZEND_FUNCTION(db2_cursor_type);
ZEND_FUNCTION(db2_rollback);
ZEND_FUNCTION(db2_free_stmt);
ZEND_FUNCTION(db2_result);
ZEND_FUNCTION(db2_fetch_row);
ZEND_FUNCTION(db2_fetch_assoc);
ZEND_FUNCTION(db2_fetch_array);
ZEND_FUNCTION(db2_fetch_both);
ZEND_FUNCTION(db2_fetch_object);
ZEND_FUNCTION(db2_free_result);
ZEND_FUNCTION(db2_set_option);
ZEND_FUNCTION(db2_client_info);
ZEND_FUNCTION(db2_server_info);
ZEND_FUNCTION(db2_escape_string);
ZEND_FUNCTION(db2_lob_read);
ZEND_FUNCTION(db2_get_option);
ZEND_FUNCTION(db2_last_insert_id);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(db2_connect, arginfo_db2_connect)
	ZEND_FE(db2_commit, arginfo_db2_commit)
	ZEND_FE(db2_pconnect, arginfo_db2_pconnect)
	ZEND_FE(db2_autocommit, arginfo_db2_autocommit)
	ZEND_FE(db2_bind_param, arginfo_db2_bind_param)
	ZEND_FE(db2_close, arginfo_db2_close)
#if defined(PASE)
	ZEND_FE(db2_pclose, arginfo_db2_pclose)
#endif
	ZEND_FE(db2_column_privileges, arginfo_db2_column_privileges)
	ZEND_FALIAS(db2_columnprivileges, db2_column_privileges, arginfo_db2_columnprivileges)
	ZEND_FE(db2_columns, arginfo_db2_columns)
	ZEND_FE(db2_foreign_keys, arginfo_db2_foreign_keys)
	ZEND_FALIAS(db2_foreignkeys, db2_foreign_keys, arginfo_db2_foreignkeys)
	ZEND_FE(db2_primary_keys, arginfo_db2_primary_keys)
	ZEND_FALIAS(db2_primarykeys, db2_primary_keys, arginfo_db2_primarykeys)
	ZEND_FE(db2_procedure_columns, arginfo_db2_procedure_columns)
	ZEND_FALIAS(db2_procedurecolumns, db2_procedure_columns, arginfo_db2_procedurecolumns)
	ZEND_FE(db2_procedures, arginfo_db2_procedures)
	ZEND_FE(db2_special_columns, arginfo_db2_special_columns)
	ZEND_FALIAS(db2_specialcolumns, db2_special_columns, arginfo_db2_specialcolumns)
	ZEND_FE(db2_statistics, arginfo_db2_statistics)
	ZEND_FE(db2_table_privileges, arginfo_db2_table_privileges)
	ZEND_FALIAS(db2_tableprivileges, db2_table_privileges, arginfo_db2_tableprivileges)
	ZEND_FE(db2_tables, arginfo_db2_tables)
	ZEND_FE(db2_exec, arginfo_db2_exec)
	ZEND_FE(db2_prepare, arginfo_db2_prepare)
	ZEND_FE(db2_execute, arginfo_db2_execute)
#if !defined(PASE)
	ZEND_FE(db2_execute_many, arginfo_db2_execute_many)
#endif
	ZEND_FE(db2_stmt_errormsg, arginfo_db2_stmt_errormsg)
	ZEND_FE(db2_stmt_error, arginfo_db2_stmt_error)
	ZEND_FE(db2_conn_errormsg, arginfo_db2_conn_errormsg)
	ZEND_FE(db2_conn_error, arginfo_db2_conn_error)
	ZEND_FE(db2_next_result, arginfo_db2_next_result)
	ZEND_FE(db2_num_fields, arginfo_db2_num_fields)
	ZEND_FE(db2_num_rows, arginfo_db2_num_rows)
	ZEND_FE(db2_field_name, arginfo_db2_field_name)
	ZEND_FE(db2_field_display_size, arginfo_db2_field_display_size)
	ZEND_FE(db2_field_num, arginfo_db2_field_num)
	ZEND_FE(db2_field_precision, arginfo_db2_field_precision)
	ZEND_FE(db2_field_scale, arginfo_db2_field_scale)
	ZEND_FE(db2_field_type, arginfo_db2_field_type)
	ZEND_FE(db2_field_width, arginfo_db2_field_width)
	ZEND_FE(db2_cursor_type, arginfo_db2_cursor_type)
	ZEND_FE(db2_rollback, arginfo_db2_rollback)
	ZEND_FE(db2_free_stmt, arginfo_db2_free_stmt)
	ZEND_FE(db2_result, arginfo_db2_result)
	ZEND_FE(db2_fetch_row, arginfo_db2_fetch_row)
	ZEND_FE(db2_fetch_assoc, arginfo_db2_fetch_assoc)
	ZEND_FE(db2_fetch_array, arginfo_db2_fetch_array)
	ZEND_FE(db2_fetch_both, arginfo_db2_fetch_both)
	ZEND_FE(db2_fetch_object, arginfo_db2_fetch_object)
	ZEND_FE(db2_free_result, arginfo_db2_free_result)
	ZEND_FE(db2_set_option, arginfo_db2_set_option)
	ZEND_FALIAS(db2_setoption, db2_set_option, arginfo_db2_setoption)
	ZEND_FE(db2_client_info, arginfo_db2_client_info)
	ZEND_FE(db2_server_info, arginfo_db2_server_info)
	ZEND_FE(db2_escape_string, arginfo_db2_escape_string)
	ZEND_FE(db2_lob_read, arginfo_db2_lob_read)
	ZEND_FE(db2_get_option, arginfo_db2_get_option)
	ZEND_FALIAS(db2_getoption, db2_get_option, arginfo_db2_getoption)
	ZEND_FE(db2_last_insert_id, arginfo_db2_last_insert_id)
	ZEND_FE_END
};
