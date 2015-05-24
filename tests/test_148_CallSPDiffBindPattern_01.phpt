--TEST--
IBM-DB2: Call a stored procedure in a different binding pattern
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
	require_once('connection.inc');
	$conn = db2_connect($db,$user,$password);

	@db2_exec($conn, "drop table test_58215");
	db2_exec($conn, "create table test_58215 (id integer, breed varchar(20), name varchar(20))");

	$stmt = db2_prepare( $conn, "insert into test_58215 (id,breed,name) values (5,?,?)" );
	$p11 = 'bug 58215 1';
	$p22 = 'bug 58215 2';
	db2_bind_param( $stmt , 2 , 'p22' );
	db2_bind_param( $stmt , 1 , 'p11' );
	db2_execute( $stmt );

	$stmt = db2_prepare($conn, "select * from test_58215 where id = 5");
	db2_execute($stmt);
	$row = db2_fetch_both($stmt);
	echo $row[0] . " , " . $row[1] .  " , " . $row[2] . "\n";

	@db2_exec($conn, "drop table test_58215");
	db2_close($conn);
?>
--EXPECT--
5 , bug 58215 1 , bug 58215 2
