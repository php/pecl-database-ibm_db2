--TEST--
IBM-DB2: set autocommit value in connection string
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$options = array(
  'autocommit' => DB2_AUTOCOMMIT_OFF
                );

$conn = db2_connect($database, $user, $password, $options);

$ac = db2_autocommit( $conn );

echo $ac;

?>
--EXPECT--
0
