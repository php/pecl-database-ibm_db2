--TEST--
IBM-DB2: Testing Graphic Datatype.
--SKIPIF--
<?php require_once('skipif3.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if($conn) {
	echo "Connection created.\n";
	
	/* Required SQls. */
	$sql_drop_table = "DROP TABLE test_graphic";
	$sql_create_table = "CREATE TABLE test_graphic (TEXT_F CHARACTER(20) NOT NULL, UNICODE GRAPHIC(20))";	
	$sql_insert = "INSERT INTO test_graphic(TEXT_F, UNICODE) VALUES(?, ?)";
	$sql_select = "SELECT * FROM test_graphic";

	/* Dropping table if exist. */
	@db2_exec($conn, $sql_drop_table);
	
	/* Creating table. */
	$ret = db2_exec($conn, $sql_create_table);
	if(!$ret) {
		echo db2_stmt_errormsg() . "\n";
	}
	
	$params = array(
		array('a', "Test_01"),
		array('b', "Test_02"),
		array('c', "Test_03"),
		array('d', "Test_04"),
		array('e', "Test_05")
	);

	/* Prepare sql to insert data in table. */
	$stmt = db2_prepare($conn, $sql_insert);
	if(!$stmt) {
		echo db2_stmt_errormsg() . "\n";
	} else {
		foreach ($params as $param) {
			db2_execute($stmt, $param);
		}

		$stmt = db2_exec($conn, $sql_select);
		if(!$stmt) {
			echo db2_stmt_errormsg() . "\n";
		} else {
			while ($row = db2_fetch_array($stmt)) {
				echo $row[0] . "|";
				echo $row[1] . "|";
				echo "\n";
			}
		}
	}

	db2_close($conn);
	echo "Connection closed.";
} else {
	echo "Connection Failed.\n";
	echo db2_conn_errormsg() . "\n";
}
?>
--EXPECT--
Connection created.
a                   |Test_01             |
b                   |Test_02             |
c                   |Test_03             |
d                   |Test_04             |
e                   |Test_05             |
Connection closed.
