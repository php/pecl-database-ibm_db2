--TEST--
IBM-DB2: db2_field_type() - by position
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");
$result2 = db2_exec($conn, "select * from staff");
$result3 = db2_exec($conn, "select * from emp_photo");

for ($i=0; $i <= db2_num_fields($result); $i++) 
{
   print $i . ":" . db2_field_type($result,$i) . "\n";
}
print "\n-----\n";
for ($i=0; $i < db2_num_fields($result2); $i++) 
{
   print $i . ":" . db2_field_type($result2,$i) . "\n";
}
print "\n-----\n";
for ($i=0; $i < db2_num_fields($result3); $i++) 
{
   print $i . ":" . db2_field_type($result3,$i) . "\n";
}
print "\n-----\n";

print "region:" . db2_field_type($result,'region') . "\n";
print "5:" . db2_field_type($result2,5) . "\n";

?>
--EXPECT--
0:date
1:string
2:string
3:int
4:

-----
0:int
1:string
2:int
3:string
4:int
5:real
6:real

-----
0:string
1:string
2:blob

-----
region:
5:real
