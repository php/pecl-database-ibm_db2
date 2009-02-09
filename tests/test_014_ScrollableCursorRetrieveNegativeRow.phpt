--TEST--
IBM-DB2: scrollable cursor; retrieve negative row
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db, $username, $password);
$query = 'select * from animals order by name';
$stmt = db2_prepare( $conn, $query, array('CURSOR' => DB2_SCROLLABLE) );
db2_execute($stmt);
$rc = db2_fetch_row($stmt,-1);
printf( "\nFetch row -1: %s\n", $rc ? "true" : "false" );

db2_close($conn);
?>
--EXPECTREGEX--
(PHP )?Warning:\s+db2_fetch_row\(\): Requested row number must be a positive value in .*?test_014_ScrollableCursorRetrieveNegativeRow.php on line 9

Fetch row -1: false
