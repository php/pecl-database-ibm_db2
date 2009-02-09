--TEST--
IBM-DB2: get autocommit value
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$ac = db2_autocommit( $conn );

echo $ac;

?>
--EXPECT--
1
