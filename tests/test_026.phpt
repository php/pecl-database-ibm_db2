--TEST--
IBM-DB2: db2_bind_param -- test db2_bind_param for a clean exit
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
	$sql = 'CALL match_animal(?, ?, ?)';
	$stmt = db2_prepare($conn, $sql);

	$name = "Peaches";
	$second_name = "Rickety Ride";
	$weight = 0;
	db2_bind_param($stmt, 1, "name", DB2_PARAM_IN);
	db2_bind_param($stmt, 2, "second_name", DB2_PARAM_INOUT);
	db2_bind_param($stmt, 3, "weight", DB2_PARAM_OUT);

	echo "This script should exit cleanly with no segfaults";
}

?>
--EXPECT--
This script should exit cleanly with no segfaults
