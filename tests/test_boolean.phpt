--TEST--
IBM-DB2: Boolean data type test.
--SKIPIF--
<?php

require_once('skipif.inc');

// This test requires IBM i 7.5 or a version of LUW that supports boolean
require_once('connection.inc');
$conn = db2_connect($database, $user, $password);
if (!$conn) {
	die("skip can't connect to DB:" . db2_conn_errormsg());
}

$info = db2_server_info($conn);
if ($info->DBMS_NAME == "AS") { // IBM i
	// DBMS_VER is VVRRM string
	$major = $info[1];
	$minor = $info[3];
	$mod = $info[4];
	if (!version_compare("$major.$minor.$mod", "7.5.0", ">=")) {
		die("skip IBM i version too old");
	}
}
// XXX: LUW, z

?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if (!$conn) {
	echo "Error connecting to database " . db2_conn_errormsg() ."\n";
	exit;
}

$s = db2_exec($conn, "values (true, false)");
$r = db2_fetch_array($s);
var_dump($r);

$s = db2_exec($conn, "values (true, false)");
db2_fetch_row($s);
var_dump(db2_result($s, 0));
var_dump(db2_result($s, 1));
echo db2_field_type($s, 1) . "\n";

db2_close($conn);

?>
--EXPECT--
array(2) {
  [0]=>
  bool(true)
  [1]=>
  bool(false)
}
bool(true)
bool(false)
boolean
