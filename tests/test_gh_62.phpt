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
$create = "CREATE OR REPLACE TABLE $table (TEXTME NVARCHAR(1024))"; /* CCSID 1200? */
$res = db2_exec($conn, $create);
$insert = "INSERT INTO $table (TEXTME) VALUES (?)";
$sth = db2_prepare($conn, $insert);
$res = db2_execute($sth, array($input));
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

