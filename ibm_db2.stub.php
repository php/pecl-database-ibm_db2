<?php

/**
 * Stub for arginfo in PHP 8
 * XXX: How do we represent optionals without defaults?
 * @generate-function-entries
 * @generate-legacy-arginfo 70000
 */

/**
 * @param resource $connection
 * @return resource|false
 */
function db2_connect(#[\SensitiveParameter] string $database, ?string $username, #[\SensitiveParameter] ?string $password, array $options = []) {}

/**
 * @param resource $connection
 */
function db2_commit($connection): bool {}

/**
 * @param resource $connection
 * @return resource|false
 */
function db2_pconnect(#[\SensitiveParameter] string $database, ?string $username, #[\SensitiveParameter] ?string $password, array $options = []) {}

/**
 * @param resource $connection
 */
function db2_autocommit($connection, int $value = UNKNOWN): int|bool {}

/**
 * @param resource $stmt
 */
function db2_bind_param($stmt, int $parameter_number, string $variable_name, int $parameter_type = DB2_PARAM_IN, int $data_type = 0, int $precision = -1, int $scale = 0): bool {}

/**
 * @param resource $connection
 */
function db2_close($connection): bool {}

#ifdef PASE
/**
 * @param resource $connection
 */
function db2_pclose($connection): bool {}
#endif

/**
 * @param resource $connection
 * @return resource
 */
function db2_column_privileges($connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $column_name = null) {}

/**
 * @param resource $connection
 * @return resource
 * @alias db2_column_privileges
 */
function db2_columnprivileges($connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $column_name = null) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_columns($connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $column_name = null) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_foreign_keys($connection, ?string $qualifier, ?string $schema, string $table_name) {}

/**
 * @param resource $connection
 * @return resource
 * @alias db2_foreign_keys
 */
function db2_foreignkeys($connection, ?string $qualifier, ?string $schema, string $table_name) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_primary_keys($connection, ?string $qualifier, ?string $schema, string $table_name) {}

/**
 * @param resource $connection
 * @return resource
 * @alias db2_primary_keys
 */
function db2_primarykeys($connection, ?string $qualifier, ?string $schema, string $table_name) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_procedure_columns($connection, ?string $qualifier, string $schema, string $procedure, ?string $parameter) {}

/**
 * @param resource $connection
 * @return resource
 * @alias db2_procedure_columns
 */
function db2_procedurecolumns($connection, ?string $qualifier, string $schema, string $procedure, ?string $parameter) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_procedures($connection, ?string $qualifier, string $schema, string $procedure) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_special_columns($connection, ?string $qualifier, string $schema, string $table_name, int $scope) {}

/**
 * @param resource $connection
 * @return resource
 * @alias db2_special_columns
 */
function db2_specialcolumns($connection, ?string $qualifier, string $schema, string $table_name, int $scope) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_statistics($connection, ?string $qualifier, ?string $schema, string $table_name, bool $unique) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_table_privileges($connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null) {}

/**
 * @param resource $connection
 * @return resource
 * @alias db2_table_privileges
 */
function db2_tableprivileges($connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null) {}

/**
 * @param resource $connection
 * @return resource
 */
function db2_tables($connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $table_type = null) {}

/**
 * @param resource $connection
 * @return resource|false
 */
function db2_exec($connection, string $statement, array $options = []) {}

/**
 * @param resource $connection
 * @return resource|false
 */
function db2_prepare($connection, string $statement, array $options = []) {}

/**
 * @param resource $stmt
 */
function db2_execute($stmt, array $parameters = []): bool {}

#ifndef PASE
/* XXX: Not documented and not supported on PASE. */

/**
 * @param resource $stmt
 */
function db2_execute_many($stmt, array $options = []): int|false {}
#endif

/**
 * @param resource|null $stmt
 */
function db2_stmt_errormsg($stmt = null): string {}

/**
 * @param resource|null $stmt
 */
function db2_stmt_error($stmt = null): string {}

/**
 * @param resource|null $connection
 */
function db2_conn_errormsg($connection = null): string {}

/**
 * @param resource|null $connection
 */
function db2_conn_error($connection = null): string {}

/**
 * @param resource $stmt
 * @return resource|false
 */
function db2_next_result($stmt) {}

/**
 * @param resource $stmt
 */
function db2_num_fields($stmt): int|false {}

/**
 * @param resource $stmt
 */
function db2_num_rows($stmt): int|false {}

/**
 * @param resource $stmt
 */
function db2_field_name($stmt, int|string $column): string|false {}

/**
 * @param resource $stmt
 */
function db2_field_display_size($stmt, int|string $column): int|false {}

/**
 * @param resource $stmt
 */
function db2_field_num($stmt, int|string $column): int|false {}

/**
 * @param resource $stmt
 */
function db2_field_precision($stmt, int|string $column): int|false {}

/**
 * @param resource $stmt
 */
function db2_field_scale($stmt, int|string $column): int|false {}

/**
 * @param resource $stmt
 */
function db2_field_type($stmt, int|string $column): string|false {}

/**
 * @param resource $stmt
 */
function db2_field_width($stmt, int|string $column): int|false {}

/**
 * @param resource $stmt
 */
function db2_cursor_type($stmt): int {}

/**
 * @param resource $connection
 */
function db2_rollback($connection): bool {}

/**
 * @param resource $stmt
 */
function db2_free_stmt($stmt): bool {}

/**
 * @param resource $stmt
 */
function db2_result($stmt, int|string $column): mixed|null {}

/**
 * @param resource $stmt
 */
function db2_fetch_row($stmt, int $row_number = -1): bool {}

/**
 * @param resource $stmt
 */
function db2_fetch_assoc($stmt, int $row_number = -1): array|false {}

/**
 * @param resource $stmt
 */
function db2_fetch_array($stmt, int $row_number = -1): array|false {}

/**
 * @param resource $stmt
 */
function db2_fetch_both($stmt, int $row_number = -1): array|false {}

/**
 * @param resource $stmt
 */
function db2_fetch_object($stmt, int $row_number = -1): \stdClass|false {}

/**
 * @param resource $stmt
 */
function db2_free_result($stmt): bool {}

/**
 * @param resource $resource
 */
function db2_set_option($resource, array $options, int $type): bool {}

/**
 * @param resource $resource
 * @alias db2_set_option
 */
function db2_setoption($resource, array $options, int $type): bool {}

/**
 * @param resource $connection
 */
function db2_client_info($connection): \stdClass|false {}

/**
 * @param resource $connection
 */
function db2_server_info($connection): \stdClass|false {}

function db2_escape_string(string $string_literal): string {}

/**
 * @param resource $stmt
 */
function db2_lob_read($stmt, int $colnum, int $length): string|false {}

/**
 * @param resource $resource
 */
function db2_get_option($resource, string $option): string|false {}

/**
 * @param resource $resource
 * @alias db2_get_option
 */
function db2_getoption($resource, string $option): string|false {}

/**
 * @param resource $resource
 */
function db2_last_insert_id($resource): ?string {}
