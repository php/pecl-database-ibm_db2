--TEST--
IBM-DB2: set autocommit with db2_autocommit
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

db2_autocommit( $conn, 0 );

$ac = db2_autocommit( $conn );

echo $ac;

?>
--EXPECT--
0
