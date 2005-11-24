--TEST--
IBM-DB2: scrollable cursor; retrieve negative row
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db, $username, $password);
$query = 'select * from animals order by name';
$res = db2_exec( $conn, $query, array('CURSOR' => DB2_SCROLLABLE) );
$rows = db2_num_rows($res);
echo $rows;
$rc = db2_fetch_row($res,-1);
     printf( "\nFetch row -1: %s\n", $rc ? "true" : "false" );

db2_close($conn);
?>
--EXPECT--
7
Fetch row -1: false
