--TEST--
IBM-DB2: Call a stored procedure in a different binding pattern
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
	require_once('connection.inc');
	$conn = db2_connect($db,$user,$password);

	// Set up //
	$sql = "DROP TABLE sptb";
	@db2_exec( $conn, $sql );
	$sql = "DROP PROCEDURE sp";
	@db2_exec( $conn, $sql );
	$sql = "CREATE TABLE sptb (c1 INTEGER, c2 FLOAT, c3 VARCHAR(10), c4 BIGINT, c5 CLOB)";
	db2_exec( $conn, $sql );
	$sql = "INSERT INTO sptb (c1, c2, c3, c4, c5) VALUES
		(1, 5.01, 'varchar', 3271982, 'clob data clob data')";
	db2_exec( $conn, $sql );
	$sql = "CREATE PROCEDURE sp(OUT out1 INTEGER, OUT out2 FLOAT, OUT out3 VARCHAR(10), OUT out4 BIGINT, OUT out5 CLOB)
			DYNAMIC RESULT SETS 1 LANGUAGE SQL BEGIN
			SELECT c1, c2, c3, c4, c5 INTO out1, out2, out3, out4, out5 FROM sptb; END";
	db2_exec($conn, $sql);

	// Run the test //
	$stmt = db2_prepare( $conn , "CALL sp(?, ?, ?, ?, ?)" );

	$out1 = 0;
	$out2 = 0.00;
	$out3 = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
	$out4 = 0;
	$out5 = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";

	db2_bind_param( $stmt , 1 , "out1" , DB2_PARAM_OUT );
	db2_bind_param( $stmt , 2 , "out2" , DB2_PARAM_OUT );
	db2_bind_param( $stmt , 3 , "out3" , DB2_PARAM_OUT );
	db2_bind_param( $stmt , 4 , "out4" , DB2_PARAM_OUT );
	db2_bind_param( $stmt , 5 , "out5" , DB2_PARAM_OUT , DB2_CLOB );

	$result = db2_execute( $stmt );

	echo "out 1:\n";
	echo $out1 . "\n";
	echo "out 2:\n";
	echo $out2 . "\n";
	echo "out 3:\n";
	echo $out3 . "\n";
	echo "out 4:\n";
	echo $out4 . "\n";
	echo "out 5:\n";
	echo $out5 . "\n";
?>
--EXPECT--
out 1:
1
out 2:
5.01
out 3:
varchar
out 4:
3271982
out 5:
clob data clob data

