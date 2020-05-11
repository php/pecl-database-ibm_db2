--TEST--
IBM-DB2: 1.9.7 - IBM i max use pconnect (operator issue)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');

putenv("IBM_DB_I5_TEST=1");

putenv("IBM_DB_i5_max_pconnect=2");
$conn1 = db2_pconnect($database, $user, $password);
$xml = xmlservice_diag($conn1);
$key1 = xmlservice_diag_jobinfo($xml);
echo "$key1\n";

$conn2 = db2_pconnect($database, $user, $password);
$xml = xmlservice_diag($conn2);
$key2 = xmlservice_diag_jobinfo($xml);
echo "$key2\n";

if ($key1 == $key2) {
  echo "success\n";
} else {
  echo "failed ($key1 == $key2)\n";
}

$conn3 = db2_pconnect($database, $user, $password);
$xml = xmlservice_diag($conn3);
$key3 = xmlservice_diag_jobinfo($xml);
echo "$key3\n";

if ($key3 == $key2) {
  echo "failed ($key3 == $key2)\n";
} else {
  echo "success\n";
}


?>
--EXPECTF--
%s
%s
success
%s
success

