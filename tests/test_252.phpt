--TEST--
IBM-DB2: db2_free_result() - 3
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

@db2_exec($conn, "drop table test_58210");
db2_exec($conn, "create table test_58210 (id integer, breed varchar(10))");

$stmt = db2_prepare( $conn, "insert into test_58210 (id, breed) values (?, ?)" );
$id = 1;
$breed = "Java";
db2_bind_param( $stmt, 1, "id" );
db2_bind_param( $stmt, 2, "breed" );
db2_execute( $stmt );
db2_free_result( $stmt );

$stmt = db2_prepare( $conn, "select * from test_58210" );
db2_execute( $stmt );
$row = db2_fetch_both( $stmt );
echo $row[0] . " , " . $row[1];

?>
--EXPECT--
1 , Java
