--TEST--
IBM-DB2: PECL bug 6792 -- db2_field_type returns 'string' for date, time, and timestamp
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
	$statement = "DROP TABLE table_6792";
	$result = @db2_exec($conn, $statement);
	$statement = "CREATE TABLE table_6792 (col1 TIME, col2 DATE, col3 TIMESTAMP)";
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO table_6792 (col1, col2, col3) values ('10.42.34', '1981-07-08', '1981-07-08-10.42.34')";
	$result = db2_exec($conn, $statement);
	$statement = "SELECT * FROM table_6792";
	$result = db2_exec($conn, $statement);
	
	for ($i=0; $i < db2_num_fields($result); $i++) 
	{
	   echo $i . ":" . db2_field_type($result,$i) . "\n";
	}

	$statement = "SELECT * FROM table_6792";
	$stmt = db2_prepare($conn, $statement);
	$rc = db2_execute($stmt);
	while(db2_fetch_row($stmt)) {
		$row0 = db2_result($stmt, 0);
		$row1 = db2_result($stmt, 1);
		$row2 = db2_result($stmt, 2);
		echo $row0 . "\n";
		echo $row1 . "\n";
		echo $row2 . "\n";
	}
	
	$statement = "DROP TABLE table_6792";
	$result = @db2_exec($conn, $statement);

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
10:42:34
1981-07-08
1981-07-08 10:42:34.000000

