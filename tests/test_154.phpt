--TEST--
IBM-DB2: db2_fetch_both(), db2_fetch_array(), db2_fetch_assoc() - Test all fetches
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

if ($conn) {
	$statement = 'DROP TABLE fetch_test';
	$result = @db2_exec($conn, $statement);
	$statement = 'CREATE TABLE fetch_test (col1 VARCHAR(20), col2 CLOB(20), col3 INTEGER)';
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO fetch_test VALUES ('column 0', 'Data in the clob 0', 0)";
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO fetch_test VALUES ('column 1', 'Data in the clob 1', 1)";
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO fetch_test VALUES ('column 2', 'Data in the clob 2', 2)";
	$result = db2_exec($conn, $statement);
	$statement = "INSERT INTO fetch_test VALUES ('column 3', 'Data in the clob 3', 3)";
	$result = db2_exec($conn, $statement);
	$statement = "SELECT col1, col2 FROM fetch_test";

	$result = db2_prepare($conn, $statement);
	db2_execute($result);

	while ($row = db2_fetch_array($result)) {
		printf ("\"%s\" from VARCHAR is %d bytes long, \"%s\" from CLOB is %d bytes long.\n",
                $row[0], strlen($row[0]),
                $row[1], strlen($row[1]));
	}

	$result = db2_prepare($conn, $statement);
	db2_execute($result);

	while ($row = db2_fetch_assoc($result)) {
		printf ("\"%s\" from VARCHAR is %d bytes long, \"%s\" from CLOB is %d bytes long.\n",
                $row['COL1'], strlen($row['COL1']),
                $row['COL2'], strlen($row['COL2']));
	}

	$result = db2_prepare($conn, $statement);
	db2_execute($result);

	while ($row = db2_fetch_both($result)) {
		printf ("\"%s\" from VARCHAR is %d bytes long, \"%s\" from CLOB is %d bytes long.\n",
                $row['COL1'], strlen($row['COL1']),
                $row[1], strlen($row[1]));
	}

	$statement = 'DROP TABLE fetch_test';
	$result = @db2_exec($conn, $statement);
	db2_close($conn);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
"column 0" from VARCHAR is 8 bytes long, "Data in the clob 0" from CLOB is 18 bytes long.
"column 1" from VARCHAR is 8 bytes long, "Data in the clob 1" from CLOB is 18 bytes long.
"column 2" from VARCHAR is 8 bytes long, "Data in the clob 2" from CLOB is 18 bytes long.
"column 3" from VARCHAR is 8 bytes long, "Data in the clob 3" from CLOB is 18 bytes long.
"column 0" from VARCHAR is 8 bytes long, "Data in the clob 0" from CLOB is 18 bytes long.
"column 1" from VARCHAR is 8 bytes long, "Data in the clob 1" from CLOB is 18 bytes long.
"column 2" from VARCHAR is 8 bytes long, "Data in the clob 2" from CLOB is 18 bytes long.
"column 3" from VARCHAR is 8 bytes long, "Data in the clob 3" from CLOB is 18 bytes long.
"column 0" from VARCHAR is 8 bytes long, "Data in the clob 0" from CLOB is 18 bytes long.
"column 1" from VARCHAR is 8 bytes long, "Data in the clob 1" from CLOB is 18 bytes long.
"column 2" from VARCHAR is 8 bytes long, "Data in the clob 2" from CLOB is 18 bytes long.
"column 3" from VARCHAR is 8 bytes long, "Data in the clob 3" from CLOB is 18 bytes long.

