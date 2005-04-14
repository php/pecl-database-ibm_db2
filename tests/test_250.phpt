--TEST--
IBM-DB2: db2_free_result() - 1
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");
$result2 = db2_exec($conn, "select * from staff");
$result3 = db2_exec($conn, "select * from emp_photo");

$r1 = db2_free_result($result);
$r2 = db2_free_result($result2);
$r3 = db2_free_result($result3);

var_dump( $r1 );
var_dump( $r2 );
var_dump( $r3 );

?>
--EXPECT--
bool(true)
bool(true)
bool(true)