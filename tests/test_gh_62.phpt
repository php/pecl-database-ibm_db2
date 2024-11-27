--TEST--
IBM-DB2: Binding empty string to NVARCHAR (GH-62)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$input = '';
$table = "EMPTY_NVARCHAR";
$drop = "DROP TABLE $table";
$res = @db2_exec($conn, $drop);
/* ensure that SQL_NTS is used so empty strings work for not just VARCHAR */
$create = "CREATE TABLE $table (TEXTME NVARCHAR(1024))"; /* CCSID 1200? */
$res = db2_exec($conn, $create);
if ($res == false) {
	die("Failed to create table: " . db2_stmt_error($create) . " - " . db2_stmt_errormsg($create) . "\n");
}
$insert = "INSERT INTO $table (TEXTME) VALUES (?)";
$sth = db2_prepare($conn, $insert);
$res = db2_execute($sth, array($input));
if ($res == false) {
	die("Failed to insert: " . db2_stmt_error($insert) . " - " . db2_stmt_errormsg($insert) . "\n");
}
$look = "select TEXTME from $table";
$res = db2_exec($conn, $look);
$row = db2_fetch_array($res);

if (bin2hex($row[0]) == bin2hex($input)) {
	echo "success\n";
} else {
	echo "Failure\n";
}

?>
--EXPECTF--
success

