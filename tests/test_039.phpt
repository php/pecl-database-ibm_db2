--TEST--
IBM-DB2: db2_fetch_row() (index by position) - 3 - nested
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from animals", array('cursor' => DB2_SCROLLABLE) );
$i= db2_num_rows($result)-1;
while ($row = db2_fetch_row($result,$i)) {
	$result2 = db2_exec($conn, "select * from animals", array ('cursor' => DB2_SCROLLABLE));
	$j= db2_num_rows($result2)-1;
	print ",\n";
	while ($row2 = db2_fetch_row($result2,$j)) {
		print "$i)$j ";	
		$j--;
	}
print "$i, \n";
$i--;
}

?>
--EXPECT--
,
6)6 6)5 6)4 6)3 6)2 6)1 6)0 6)-1 6)-2 6)-3 6)-4 6)-5 6, 
,
5)6 5)5 5)4 5)3 5)2 5)1 5)0 5)-1 5)-2 5)-3 5)-4 5)-5 5, 
,
4)6 4)5 4)4 4)3 4)2 4)1 4)0 4)-1 4)-2 4)-3 4)-4 4)-5 4, 
,
3)6 3)5 3)4 3)3 3)2 3)1 3)0 3)-1 3)-2 3)-3 3)-4 3)-5 3, 
,
2)6 2)5 2)4 2)3 2)2 2)1 2)0 2)-1 2)-2 2)-3 2)-4 2)-5 2, 
,
1)6 1)5 1)4 1)3 1)2 1)1 1)0 1)-1 1)-2 1)-3 1)-4 1)-5 1, 
,
0)6 0)5 0)4 0)3 0)2 0)1 0)0 0)-1 0)-2 0)-3 0)-4 0)-5 0, 
,
-1)6 -1)5 -1)4 -1)3 -1)2 -1)1 -1)0 -1)-1 -1)-2 -1)-3 -1)-4 -1)-5 -1, 
,
-2)6 -2)5 -2)4 -2)3 -2)2 -2)1 -2)0 -2)-1 -2)-2 -2)-3 -2)-4 -2)-5 -2, 
,
-3)6 -3)5 -3)4 -3)3 -3)2 -3)1 -3)0 -3)-1 -3)-2 -3)-3 -3)-4 -3)-5 -3, 
,
-4)6 -4)5 -4)4 -4)3 -4)2 -4)1 -4)0 -4)-1 -4)-2 -4)-3 -4)-4 -4)-5 -4, 
,
-5)6 -5)5 -5)4 -5)3 -5)2 -5)1 -5)0 -5)-1 -5)-2 -5)-3 -5)-4 -5)-5 -5,
