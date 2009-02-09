--TEST--
IBM-DB2: db2_fetch_row() - several rows
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from staff");
$i=0;

while ($row = db2_fetch_row($result)) 
{
   print "$i, \n";
   $i++;
}
db2_close($conn);
?>
--EXPECT--
0, 
1, 
2, 
3, 
4, 
5, 
6, 
7, 
8, 
9, 
10, 
11, 
12, 
13, 
14, 
15, 
16, 
17, 
18, 
19, 
20, 
21, 
22, 
23, 
24, 
25, 
26, 
27, 
28, 
29, 
30, 
31, 
32, 
33, 
34, 
