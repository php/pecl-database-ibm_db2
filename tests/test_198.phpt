--TEST--
IBM-DB2: db2_lob_read() -- Reading and writing lobs from memory
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

if ($conn) {
	$drop = 'DROP TABLE clob_stream';
	$result = @db2_exec( $conn, $drop );

	$create = 'CREATE TABLE clob_stream (id INTEGER, my_clob CLOB)';
	$result = db2_exec( $conn, $create );

	$variable = "";
	$stmt = db2_prepare($conn, "INSERT INTO clob_stream (id,my_clob) VALUES (1, ?)");
	$variable = "THIS IS A CLOB TEST. THIS IS A CLOB TEST.";
	db2_bind_param($stmt, 1, "variable", DB2_PARAM_IN);
	db2_execute($stmt);

	$sql = "SELECT id,my_clob FROM clob_stream";
	$result = db2_prepare($conn, $sql);
	db2_execute($result);
	db2_fetch_row($result);
	$i = 0;
	while ($data = db2_lob_read($result, 2, 6)) {
		echo "Loop $i: $data\n";
		$i = $i + 1;
	}

	$drop = 'DROP TABLE blob_stream';
	$result = @db2_exec( $conn, $drop );

	$create = 'CREATE TABLE blob_stream (id INTEGER, my_blob CLOB)';
	$result = db2_exec( $conn, $create );

	$variable = "";
	$stmt = db2_prepare($conn, "INSERT INTO blob_stream (id,my_blob) VALUES (1, ?)");
	$variable = "THIS IS A BLOB TEST. THIS IS A BLOB TEST.";
	db2_bind_param($stmt, 1, "variable", DB2_PARAM_IN);
	db2_execute($stmt);

	$sql = "SELECT id,my_blob FROM blob_stream";
	$result = db2_prepare($conn, $sql);
	db2_execute($result);
	db2_fetch_row($result);
	$i = 0;
	while ($data = db2_lob_read($result, 2, 6)) {
		echo "Loop $i: $data\n";
		$i = $i + 1;
	}
} else {
   echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECTF--
Loop 0: THIS I
Loop 1: S A CL
Loop 2: OB TES
Loop 3: T. THI
Loop 4: S IS A
Loop 5:  CLOB 
Loop 6: TEST.
Loop 0: THIS I
Loop 1: S A BL
Loop 2: OB TES
Loop 3: T. THI
Loop 4: S IS A
Loop 5:  BLOB 
Loop 6: TEST.

