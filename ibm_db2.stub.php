<?php

/** @generate-function-entries */

function db2_connect(string $database, ?string $username, ?string $password, array $options = []): resource|false {}

function db2_commit(resource $connection): bool {}

function db2_pconnect(string $database, ?string $username, ?string $password, array $options = []): resource|false {}

function db2_autocommit(resource $connection, ?int $value = null): int|bool {}

function db2_bind_param(resource $connection, int $parameter_number, string $variable_name, ?int $parameter_type, int $data_type = 0, int $precision = -1, int $scale = 0): bool {}

function db2_close(resource $connection): bool {}

#ifdef PASE
function db2_pclose(resource $connection): bool {}
#endif

function db2_column_privileges(resource $connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $column_name = null): resource {}

function db2_columns(resource $connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $column_name = null): resource {}

function db2_foreign_keys(resource $connection, ?string $qualifier, ?string $schema, string $table_name): resource {}

function db2_primary_keys(resource $connection, ?string $qualifier, ?string $schema, string $table_name): resource {}

function db2_procedure_columns(resource $connection, ?string $qualifier, string $schema, string $procedure, ?string $parameter): resource {}

function db2_procedures(resource $connection, ?string $qualifier, string $schema, string $procedure): resource {}

function db2_special_columns(resource $connection, ?string $qualifier, string $schema, string $table_name, int $scope): resource {}

function db2_statistics(resource $connection, ?string $qualifier, ?string $schema, string $table_name, bool|int $unique): resource {}

function db2_table_privileges(resource $connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null): resource {}

function db2_tables(resource $connection, ?string $qualifier = null, ?string $schema = null, ?string $table_name = null, ?string $table_type = null): resource {}

function db2_exec(resource $connection, string $statement, array $options = []): resource|false {}

function db2_prepare(resource $connection, string $statement, array $options = []): resource|false {}

function db2_execute(resource $stmt, array $options = []): bool {}

#ifndef PASE
/* XXX: Not documented and not supported on PASE. */
function db2_execute_many(resource $stmt, array $options = []): int|false {}
#endif

function db2_stmt_errormsg(?resource $stmt = null): string {}

function db2_stmt_error(?resource $stmt = null): string {}

function db2_conn_errormsg(?resource $connection = null): string {}

function db2_conn_error(?resource $connection = null): string {}

function db2_next_result(resource $stmt): resource|false {}

function db2_num_fields(resource $stmt): int|false {}

function db2_num_rows(resource $stmt): int|false {}

function db2_field_name(resource $stmt, int|string $column): string|false {}

function db2_field_display_size(resource $stmt, int|string $column): int|false {}

function db2_field_num(resource $stmt, int|string $column): int|false {}

function db2_field_precision(resource $stmt, int|string $column): int|false {}

function db2_field_scale(resource $stmt, int|string $column): int|false {}

function db2_field_type(resource $stmt, int|string $column): string|false {}

function db2_field_width(resource $stmt, int|string $column): int|false {}

function db2_cursor_type(resource $stmt): int {}

function db2_rollback(resource $connection): bool {}

function db2_free_stmt(resource $stmt): bool {}

function db2_result(resource $stmt, int|string $column): object|null {}

function db2_fetch_row(resource $stmt, ?int $row_number = null): bool {}

function db2_fetch_assoc(resource $stmt, ?int $row_number = null): array|false {}

function db2_fetch_array(resource $stmt, ?int $row_number = null): array|false {}

function db2_fetch_both(resource $stmt, ?int $row_number = null): array|false {}

function db2_fetch_object(resource $stmt, ?int $row_number = null): object|false {}

function db2_free_result(resource $stmt): bool {}

function db2_set_option(resource $resource, array $options, int $type): bool {}

function db2_client_info(resource $connection): object|false {}

function db2_server_info(resource $connection): object|false {}

function db2_escape_string(string $string_literal): string {}

function db2_lob_read(resource $stmt, int $colnum, int $length): string|false {}

function db2_get_option(resource $stmt, string $option): string|false {}

function db2_last_insert_id(resource $stmt): string {}
