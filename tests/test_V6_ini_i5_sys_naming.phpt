--TEST--
IBM-DB2: 1.9.7 - IBM i system naming (operator override)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');

/*
 * For cases where we don't have $user set (IBM i implicit),
 * use the PASE username
 */
$lib = $user;
if ($lib == '') {
  /*
   * POSIX modules won't be loaded in test env and get_current_user returns
   * the file owner under POSIX
   */
  $lib = strtoupper(getenv("LOGNAME"));
}

putenv("IBM_DB_I5_TEST=1");

putenv("IBM_DB_i5_sys_naming=0");
echo("IBM_DB_i5_sys_naming=0\n");
$conn1 = db2_connect($database, $user, $password);
$stmt = @db2_exec( $conn1, "SELECT * FROM $lib/animals ORDER BY breed" );
if ($stmt) {
  echo "failed ($stmt)\n";
} else {
  echo "success\n";
}

$conn2 = db2_connect($database, $user, $password);
$stmt = db2_exec($conn2, "SELECT * FROM $lib.animals ORDER BY breed" );
if ($stmt) {
  echo "success\n";
} else {
  echo "failed ($stmt)\n";
}

putenv("IBM_DB_i5_sys_naming=1");
echo("IBM_DB_i5_sys_naming=1\n");
$conn1 = db2_connect($database, $user, $password);
$stmt = db2_exec( $conn1, "SELECT * FROM $lib/animals ORDER BY breed" );
if ($stmt) {
  echo "success\n";
} else {
  echo "failed ($stmt)\n";
}

$conn2 = db2_connect($database, $user, $password);
$stmt = db2_exec($conn2, "SELECT * FROM $lib.animals ORDER BY breed" );
if ($stmt) {
  echo "success\n";
} else {
  echo "failed ($stmt)\n";
}

?>
--EXPECTF--
%s
success
success
%s
success
success

