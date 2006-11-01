--TEST--
IBM-DB2: db2_foreign_keys -- tests the db2_foreign_keys functionality
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn != 0)
{
	$statement = 'CREATE TABLE test_primary_keys (id INTEGER NOT NULL, PRIMARY KEY(id))';
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO test_primary_keys VALUES (1)";
	$result = db2_exec($conn, $statement);
	$statement = 'CREATE TABLE test_foreign_keys (idf INTEGER NOT NULL, FOREIGN KEY(idf) REFERENCES test_primary_keys(id))';
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO test_foreign_keys VALUES (1)";
	$result = db2_exec($conn, $statement);

	$stmt = db2_foreign_keys($conn, NULL, NULL, 'TEST_PRIMARY_KEYS');
	$row = db2_fetch_array($stmt);
	echo $row[2] . "\n";
	echo $row[3] . "\n";
	echo $row[6] . "\n";
	echo $row[7] . "\n";

	$statement = 'DROP TABLE test_primary_keys';
	$result = db2_exec($conn, $statement);

	db2_close($conn);
}
else
{
	echo db2_conn_errormsg();
	printf("Connection failed\n\n");
}

?>
--EXPECT--
TEST_PRIMARY_KEYS
ID
TEST_FOREIGN_KEYS
IDF
