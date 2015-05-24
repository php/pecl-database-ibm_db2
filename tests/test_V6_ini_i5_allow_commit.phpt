--TEST--
IBM-DB2: 1.9.7 - IBM i test ini isolation modes (operator issue)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

putenv("IBM_DB_I5_TEST=1");
putenv("IBM_DB_i5_allow_commit=4");
$conn1 = db2_connect($database, $user, $password);
db2_autocommit( $conn1, DB2_AUTOCOMMIT_OFF );
$sql = 'UPDATE animals SET id = 9';
$res = db2_exec($conn1, $sql);
print "Number of affected rows: " . db2_num_rows($res). "\n";

putenv("IBM_DB_i5_allow_commit=0"); // 0,1-read uncommit; 2+ will hang
$conn2 = db2_connect($database, $user, $password);
$sql = 'select * from animals where id = 1';
$res = db2_exec($conn2, $sql);
$row = db2_fetch_array($res);
if ($row === false) {
echo "success\n";
} else {
echo "failure ($row)\n";
}
db2_close($conn2);

db2_rollback($conn1);
db2_close($conn1);
?>
--EXPECT--
Number of affected rows: 7
success

