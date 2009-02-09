--TEST--
IBM-DB2: PECL bug 6528 -- scoping problem in db2_bind_param
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

function checked_db2_execute($stmt) {
	db2_execute($stmt);
	$row = db2_fetch_array($stmt);
	var_dump($row);
}

$conn = db2_connect($database, $user, $password);

if ($conn) {
	$sql =  "SELECT RTRIM(name) FROM animals WHERE breed = ?";
	$stmt = db2_prepare( $conn, $sql );
	$var = "cat";
	db2_bind_param($stmt, 1, "var", DB2_PARAM_IN);
	checked_db2_execute($stmt);
	db2_close($conn);
}
else {
	echo "Connection failed.\n";
}

?>
--EXPECT--
array(1) {
  [0]=>
  string(4) "Pook"
}
