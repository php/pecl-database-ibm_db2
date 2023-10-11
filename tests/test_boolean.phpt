--TEST--
IBM-DB2: Boolean data type test.
--SKIPIF--
<?php

require_once('skipif.inc');

// This test requires IBM i 7.5 or DB2/LUW 9.7 for the boolean type
require_once('connection.inc');
$conn = db2_connect($database, $user, $password);
if (!$conn) {
	die("skip can't connect to DB:" . db2_conn_errormsg());
}

$info = db2_server_info($conn);
if ($info->DBMS_NAME == "AS") { // IBM i
	// DBMS_VER is VVRRM string
	$major = $info->DBMS_VER[1];
	$minor = $info->DBMS_VER[3];
	$mod = $info->DBMS_VER[4];
	if (!version_compare("$major.$minor.$mod", "7.5.0", ">=")) {
		die("skip IBM i version too old");
	}
} else { // Should cover i.e. DB2/LINUX
	// DBMS_VER is VV.RR.MMMM, version_compore should work directly
	if (!version_compare($info->DBMS_VER, "9.7.0", ">=")) {
		die("skip DB2 version too old");
	}
}
// XXX: z

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

// db2_result can't return true/false due to API limitations, so these will be ints
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
int(1)
int(0)
boolean
