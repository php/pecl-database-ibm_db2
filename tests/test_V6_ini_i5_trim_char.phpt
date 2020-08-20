--TEST--
IBM-DB2: 2.0.3 - IBM i trim char (operator override)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');

putenv("IBM_DB_I5_TEST=1");

putenv("IBM_DB_i5_char_trim=0");
echo("IBM_DB_i5_char_trim=0\n");
$conn1 = db2_connect($database, $user, $password);
$stmt = db2_exec( $conn1, "SELECT NAME,BREED FROM $user.animals where NAME='Pook'" );
while ($row = db2_fetch_array($stmt)) {
  var_dump($row);
}

putenv("IBM_DB_i5_char_trim=1");
echo("IBM_DB_i5_char_trim=1\n");
$conn1 = db2_connect($database, $user, $password);
$stmt = db2_exec( $conn1, "SELECT NAME,BREED FROM $user.animals where NAME='Pook'" );
while ($row = db2_fetch_array($stmt)) {
  var_dump($row);
}


?>
--EXPECTF--
%s
%sstring(16) "Pook            "%s
%s
%sstring(4) "Pook"%s
%s

