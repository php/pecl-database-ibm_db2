--TEST--
IBM-DB2: db2_field_width() - by position and name 2
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");
$result2 = db2_exec($conn, "select * from staff");
$result3 = db2_exec($conn, "select * from emp_photo");

for ($i=0; $i < db2_num_fields($result); $i++) 
{
   var_dump( db2_field_width($result,$i) );
}
print "\n-----\n";
for ($i=0; $i < db2_num_fields($result2); $i++) 
{
   var_dump( db2_field_width($result2,db2_field_name($result2,$i)) );
}

?>
--EXPECT--
int(10)
int(15)
int(15)
int(4)

-----
int(2)
int(9)
int(2)
int(5)
int(2)
int(1794)
int(1794)
