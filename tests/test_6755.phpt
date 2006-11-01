--TEST--
IBM-DB2: PECL bug 6755 -- Extra null character on result from CLOB column
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
	$statement = 'DROP TABLE table_6755';
	$result = @db2_exec($conn, $statement);
	$statement = 'CREATE TABLE table_6755 (col1 VARCHAR(20), col2 CLOB(20))';
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO table_6755 VALUES ('database', 'database')";
	$result = db2_exec($conn, $statement);
	$statement = "SELECT col1, col2 FROM table_6755";

	$result = db2_prepare($conn, $statement);
	db2_execute($result);

	while ($row = db2_fetch_array($result)) {
		printf ("\"%s\" from VARCHAR is %d bytes long, \"%s\" from CLOB is %d bytes long.\n",
                $row[0], strlen($row[0]),
                $row[1], strlen($row[1]));
	}
	
	$statement = 'DROP TABLE table_6755';
	$result = @db2_exec($conn, $statement);

	db2_close($conn);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
"database" from VARCHAR is 8 bytes long, "database" from CLOB is 8 bytes long.

