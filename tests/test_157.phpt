--TEST--
IBM-DB2: db2_fetch_assoc() - positioned 
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$result = db2_exec($conn, "select * from staff");

$i=1;
$row = db2_fetch_assoc($result, $i);
if ($row) {
printf ("%5d  ",$row['ID']);
printf ("%-10s ",$row['NAME']);
printf ("%5d ",$row['DEPT']);
printf ("%-7s ",$row['JOB']);
printf ("%5d ", $row['YEARS']);
printf ("%15s ", $row['SALARY']);
printf ("%10s ", $row['COMM']);
print "\n";
//$i++;
//$row = db2_fetch_assoc($result, $i);
}

?>
--EXPECT--

