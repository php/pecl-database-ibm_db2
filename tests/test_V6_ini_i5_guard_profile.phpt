--TEST--
IBM-DB2: 1.9.7 - IBM i security guard profile (customer application issue)
--SKIPIF--
<?php 
require_once('skipif.inc'); 
?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');

putenv("IBM_DB_I5_TEST=1");
putenv("IBM_DB_i5_guard_profile=1"); /* before include this test */
$conn1 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn1);
$key1 = xmlservice_diag_jobinfo($xml);
echo "key1 $key1\n";

xmlservice_switch_profile($conn1, $switch_user, $switch_password);
$xml = xmlservice_diag($conn1);
$key2 = xmlservice_diag_jobinfo($xml);
echo "key2 $key2\n";

db2_close($conn1);


putenv("IBM_DB_i5_guard_profile=1");
$conn2 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn2);
$key3 = xmlservice_diag_jobinfo($xml);
echo "key3 $key3\n";
db2_close($conn2);

/*
 * On PASE, we can use an empty user for implict same user, which
 * messes up this test, so just skip it
 */
if ($user !== "" && strpos($key1,$user) === false) {
  echo "failed key1 ($key1) missing ".$user."\n";
} else {
  echo "success\n";
}
if (strpos($key2,$switch_user) === false) {
  echo "failed key2 ($key2) missing ".$switch_user."\n";
} else {
  echo "success\n";
}
if (strpos($key3,$switch_user) === false) {
  echo "success\n";
} else {
  echo "failed key 3 ($key3) missing ".$user."\n";
}

?>
--EXPECTF--
key1 %s
key2 %s
key3 %s
success
success
success

