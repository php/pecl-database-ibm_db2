--TEST--
IBM-DB2: db2_statistics(): testing indexes
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

if ($conn) {
	$rc = @db2_exec($conn, "DROP TABLE index_test");
	$rc = db2_exec($conn, "CREATE TABLE index_test (id INTEGER, data VARCHAR(50))");
	$rc = db2_exec($conn, "CREATE INDEX index1 ON index_test (id)");

	echo "Test first index table:\n";
    $result = db2_statistics($conn,NULL,NULL,"INDEX_TEST",1);
    $row = db2_fetch_array($result);
    echo $row[2] . "\n";  // TABLE_NAME
    echo $row[3] . "\n";  // NON_UNIQUE
    echo $row[5] . "\n";  // INDEX_NAME
    echo $row[8] . "\n";  // COLUMN_NAME


	$rc = @db2_exec($conn, "DROP TABLE index_test2");
	$rc = db2_exec($conn, "CREATE TABLE index_test2 (id INTEGER, data VARCHAR(50))");
	$rc = db2_exec($conn, "CREATE INDEX index2 ON index_test2 (data)");

	echo "Test second index table:\n";
    $result = db2_statistics($conn,NULL,NULL,"INDEX_TEST2",1);
    $row = db2_fetch_array($result);
    echo $row[2] . "\n";  // TABLE_NAME
    echo $row[3] . "\n";  // NON_UNIQUE
    echo $row[5] . "\n";  // INDEX_NAME
    echo $row[8] . "\n";  // COLUMN_NAME

	echo "Test non-existent table:\n";
    $result = db2_statistics($conn,NULL,NULL,"NON_EXISTENT_TABLE",1);
    $row = db2_fetch_array($result);
    if ($row) {
       echo "Non-Empty\n";
    } else {
       echo "Empty\n";
    }

} else {
   echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECTF--
Test first index table:
INDEX_TEST
1
INDEX1
ID
Test second index table:
INDEX_TEST2
1
INDEX2
DATA
Test non-existent table:
Empty

