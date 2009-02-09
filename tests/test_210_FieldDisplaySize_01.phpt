--TEST--
IBM-DB2: db2_field_display_size() - 1
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from staff");
$cols = db2_num_fields($result);

for ($i=0; $i<$cols; $i++) 
{
   $size = db2_field_display_size($result,$i);
   print "col:$i and size: $size\n";	
}

db2_close($conn);

?>
--EXPECT--
col:0 and size: 6
col:1 and size: 9
col:2 and size: 6
col:3 and size: 5
col:4 and size: 6
col:5 and size: 9
col:6 and size: 9
