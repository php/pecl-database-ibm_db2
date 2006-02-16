--TEST--
IBM-DB2: PECL bug 6792 -- db2_field_type returns 'string' for date, time, and timestamp
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
	$statement = "CREATE TABLE table_6792 (col1 time, col2 date, col3 timestamp)";
	$result = db2_exec($conn, $statement);
	$statement = "SELECT * FROM table_6792";
	$result = db2_exec($conn, $statement);
	
	for ($i=0; $i < db2_num_fields($result); $i++) 
	{
	   echo $i . ":" . db2_field_type($result,$i) . "\n";
	}
	
	db2_close($conn);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
0:time
1:date
2:timestamp