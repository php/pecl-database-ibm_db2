--TEST--
IBM-DB2: db2_field_display_size() - 3
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");

$i = "SALES_PERSON";

printf ("%s size %d\n",$i, db2_field_display_size($result,$i));

$i=2;
printf ("%d size %d\n",$i, db2_field_display_size($result,$i));

?>
--EXPECT--
SALES_PERSON size 15
2 size 15
