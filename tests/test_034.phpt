--TEST--
IBM-DB2: db2_fetch_row()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select * from staff");

if ($row = db2_fetch_assoc($result)) 
{
   printf ("%5d  ",$row['ID']);
   printf ("%-10s ",$row['NAME']);
   printf ("%5d ",$row['DEPT']);
   printf ("%-7s ",$row['JOB']);
   printf ("%5d ", $row['YEARS']);
   printf ("%15s ", $row['SALARY']);
   printf ("%10s ", $row['COMM']);
   print "\n";
}


db2_close($conn);

?>
--EXPECT--
   10  Sanders       20 Mgr         7        18357.50            
