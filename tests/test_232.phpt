--TEST--
IBM-DB2: db2_field_type() - by position and name
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");

for ($i=0; $i <= db2_num_fields($result); $i++) 
{
   print db2_field_name($result,$i) . ":" . db2_field_type($result,db2_field_name($result,$i)) . "\n";
}
print "-----\n";

$t = db2_field_type($result,99);
var_dump( $t );

$t1 = db2_field_type($result, "HELMUT");
var_dump( $t1 );

?>
--EXPECT--
SALES_DATE:string
SALES_PERSON:string
REGION:string
SALES:int
:
-----
bool(false)
bool(false)