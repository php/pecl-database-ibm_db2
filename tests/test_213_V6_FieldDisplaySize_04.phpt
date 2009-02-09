--TEST--
IBM-DB2: db2_field_display_size() - 4
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");

$i = "sales_person";

printf ("%s size %d\n",$i, db2_field_display_size($result,$i));

$i = "REGION";

printf ("%s size %d\n",$i, db2_field_display_size($result,$i));

$i = "REgion";

printf ("%s size %d\n",$i, db2_field_display_size($result,$i));

$i = "HELMUT";

printf ("%s size %d\n",$i, db2_field_display_size($result,$i));

$t = db2_field_display_size($result,"");

var_dump( $t );

$t = db2_field_display_size($result,"HELMUT");

var_dump( $t );

$t = db2_field_display_size($result,"Region");

var_dump( $t );

$t = db2_field_display_size($result,"SALES_DATE");

var_dump( $t );

?>
--EXPECT--
sales_person size 0
REGION size 16
REgion size 0
HELMUT size 0
bool(false)
bool(false)
bool(false)
int(11)
