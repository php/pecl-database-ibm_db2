--TEST--
IBM-DB2: 1.9.7 - LUW and IBM i system naming mode DB2_I5_NAMING_ON
--SKIPIF--
<?php require_once('skipifNot10_5.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
$conn = db2_connect($database, $user, $password, array('i5_naming'=>DB2_I5_NAMING_ON, 'i5_libl'=>"QTEMP FROG", 'i5_curlib'=>"FROG"));
if (!$conn) die("connect\n".db2_stmt_errormsg()."\n");
$stmt = db2_exec( $conn, "SELECT * FROM $user/animals ORDER BY breed" );
if (!$stmt) die("stmt\n".db2_stmt_errormsg()."\n");
while ($row = db2_fetch_array($stmt)) {
echo "fetch\n".db2_stmt_errormsg()."\n";
var_dump($row);
}
echo "success\n";
?>
--EXPECTF--
%s
success

