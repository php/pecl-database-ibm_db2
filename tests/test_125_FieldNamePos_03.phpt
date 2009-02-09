--TEST--
IBM-DB2: db2_field_name() - by position 3
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from sales");
$result2 = db2_exec($conn, "select * from staff");

for ($i=0; $i < db2_num_fields($result); $i++) 
{
   print $i . ":" . db2_field_name($result,$i) . "\n";
}

print "\n-----\n";

for ($i=0; $i < db2_num_fields($result2); $i++) 
{
   print $i . ":" . db2_field_name($result2,$i) . "\n";
}

print "\n-----\n";

print "Region:" . db2_field_name($result,'REGION') . "\n";
print "5:" . db2_field_name($result2,5) . "\n";

?>
--EXPECT--
0:SALES_DATE
1:SALES_PERSON
2:REGION
3:SALES

-----
0:ID
1:NAME
2:DEPT
3:JOB
4:YEARS
5:SALARY
6:COMM

-----
Region:REGION
5:SALARY
