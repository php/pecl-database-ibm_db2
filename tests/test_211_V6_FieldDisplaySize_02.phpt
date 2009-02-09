--TEST--
IBM-DB2: db2_field_display_size() - 2
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");

$i=1;

while ($i <= db2_num_fields($result)) 
{
   printf ("%d size %d\n",$i, db2_field_display_size($result,$i));
   $i++;
}

db2_close($conn);

?>
--EXPECT--
1 size 16
2 size 16
3 size 12
4 size 0
