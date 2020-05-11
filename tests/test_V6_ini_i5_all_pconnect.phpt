--TEST--
IBM-DB2: 1.9.7 - IBM i force all connect to pconnect (operator issue)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');

putenv("IBM_DB_I5_TEST=1");

putenv("IBM_DB_i5_all_pconnect=1");
$conn1 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn1);
$key1 = xmlservice_diag_jobinfo($xml);
echo "$key1\n";

putenv("IBM_DB_i5_all_pconnect=1");
$conn2 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn2);
$key2 = xmlservice_diag_jobinfo($xml);
echo "$key2\n";

if ($key1 == $key2) {
  echo "success\n";
} else {
  echo "failed ($key1 == $key2)\n";
}


putenv("IBM_DB_i5_all_pconnect=0");
$conn3 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn3);
$key3 = xmlservice_diag_jobinfo($xml);
echo "$key3\n";

putenv("IBM_DB_i5_all_pconnect=0");
$conn4 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn4);
$key4 = xmlservice_diag_jobinfo($xml);
echo "$key4\n";

if ($key1 != $key3) {
  echo "success\n";
} else {
  echo "failed ($key1 == $key3)\n";
}

if ($key3 != $key4) {
  echo "success\n";
} else {
  echo "failed ($key3 == $key4)\n";
}



?>
--EXPECTF--
%s
%s
success
%s
%s
success
success

