--TEST--
IBM-DB2: db2_num_rows - select - scrollable cursor 2
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db, $username, $password);
$query = 'select * from staff';
$res = db2_exec( $conn, $query, array('CURSOR' => DB2_SCROLLABLE) );
$rows = db2_num_rows($res);
echo $rows;
$rc = db2_fetch_row($res,-1);
     printf( "Fetch row -1: %s\n", $rc ? "true" : "false" );

db2_close($conn);
?>
--EXPECT--
35Fetch row -1: false
