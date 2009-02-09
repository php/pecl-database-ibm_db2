--TEST--
IBM-DB2: db2_fetch_assoc() - simple select 4
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$result = db2_exec($conn, "select * from org");

while ($row = db2_fetch_assoc($result)) {

printf ("%4d ",$row['DEPTNUMB']);
printf ("%-14s ",$row['DEPTNAME']);
printf ("%4d ",$row['MANAGER']);
printf ("%-10s",$row['DIVISION']);
printf ("%-13s ",$row['LOCATION']);

print "\n";
}

?>
--EXPECT--

  10 Head Office     160 Corporate New York      
  15 New England      50 Eastern   Boston        
  20 Mid Atlantic     10 Eastern   Washington    
  38 South Atlantic   30 Eastern   Atlanta       
  42 Great Lakes     100 Midwest   Chicago       
  51 Plains          140 Midwest   Dallas        
  66 Pacific         270 Western   San Francisco 
  84 Mountain        290 Western   Denver        

