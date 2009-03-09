--TEST--
IBM-DB2: Real datatype test.
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
	// Dropping the test table, if exeist.
	$res = @db2_exec($conn, "DROP TABLE TEST1R");
	
	// Creating new test table.
	$res = @db2_exec($conn, "CREATE TABLE TEST1R (TZ_OFFSET REAL)");
	
	// Inserting values to the test table.
	$res = db2_exec($conn, "INSERT INTO TEST1R VALUES(-2.34)");
	
	$res = db2_exec($conn, "SELECT tz_offset from test1R");
	
	while ($row = db2_fetch_both($res)) {
		var_dump($row);
	}
	
	// Dropping the test table.
	$res = @db2_exec($conn, "DROP TABLE TEST1R");
	
	db2_close($conn);
} else {
	print "Connection not created" . "\n";
}
?>
--EXPECT--
array(2) {
  ["TZ_OFFSET"]=>
  float(-2.3399999141693)
  [0]=>
  float(-2.3399999141693)
}