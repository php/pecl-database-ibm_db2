--TEST--
IBM-DB2: 1.9.7 - IBM i check pconnect (alternative LUW ping)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');

putenv("IBM_DB_I5_TEST=1");

putenv("IBM_DB_i5_check_pconnect=-42");
echo "\n(IBM_DB_i5_check_pconnect=-42)\n";
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

// kill the job (pconnect still active)
$xml = xmlservice_kill($conn1);

putenv("IBM_DB_i5_check_pconnect=4");
echo "(IBM_DB_i5_check_pconnect=4)\n";
$conn3 = db2_pconnect($database, $user, $password);
if (!$conn3) die("bad connect $conn3 (IBM_DB_i5_check_pconnect=4)");
$xml = xmlservice_diag($conn3);
$key3 = xmlservice_diag_jobinfo($xml);
echo "$key3\n";

if ($key3 == $key2) {
  echo "failed ($key3 == $key2)\n";
} else {
  echo "success\n";
}

// echo $xml;

?>
--EXPECTF--
(IBM_DB_i5_check_pconnect=-42)
%s
%s
success
(IBM_DB_i5_check_pconnect=4)
%s
success

