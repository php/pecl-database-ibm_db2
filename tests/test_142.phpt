--TEST--
IBM-DB2: db2_bind_param: SELECT statement (multiple params) - DB2_PARAM_IN
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$stmt = db2_prepare( $conn, "select * from employee where salary > ? and salary < ? order by 1" );

db2_bind_param( $stmt, 1, "lower_limit", DB2_PARAM_IN );
db2_bind_param( $stmt, 2, "upper_limit", DB2_PARAM_IN );

$lower_limit = 20000.00;
$upper_limit = 40000.00;

$result = db2_execute( $stmt );

while ($row=db2_fetch_into($result)) 
{
   print $row[0] . "," . $row[1] . "\n";	
}


?>
--EXPECT--
000030,SALLY
000060,IRVING
000070,EVA
000090,EILEEN
000100,THEODORE
000120,SEAN
000130,DOLORES
000140,HEATHER
000150,BRUCE
000160,ELIZABETH
000170,MASATOSHI
000180,MARILYN
000190,JAMES
000200,DAVID
000220,JENNIFER
000230,JAMES
000240,SALVATORE
000270,MARIA
000280,ETHEL
000330,WING
000340,JASON