--TEST--
IBM-DB2: Timestamp data type test.
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if (!$conn) {
	echo "Error connecting to database " . db2_conn_errormsg() ."\n";
	exit;
}
  
// Drop/Create source and target tables
@db2_exec($conn, "DROP TABLE src");
@db2_exec($conn, "DROP TABLE targ");

db2_exec($conn, "CREATE TABLE src (c1 INT, c2 TIMESTAMP)");
db2_exec($conn, "CREATE TABLE targ (c1 INT, c2 TIMESTAMP)");

// If you insert NULL as the timestamp into the src table, the later insert
// will NOT work.
db2_exec($conn, "INSERT INTO src VALUES (NULL, '1981-05-13 12:56:46.000000')");
db2_exec($conn, "INSERT INTO src VALUES (1, NULL)");
db2_exec($conn, "INSERT INTO src VALUES (1, '1981-05-13 12:56:46.000000')");

$rs = db2_exec($conn, "SELECT c1, c2 FROM src ORDER BY c2");

$prep = db2_prepare ($conn, "INSERT INTO targ VALUES (?, ?)");

echo "Transfering data from src table to targ table:\n";
while($valuesOut = db2_fetch_array($rs)) {
	$rs1 = db2_execute($prep, $valuesOut);
	if (!$rs1) {
		echo "Error inserting into targ \n";
		var_dump($valuesOut);
		exit;
	}
}

echo "Values from src table:\n";
$rs = db2_exec($conn, "SELECT c1, c2 FROM src ORDER BY c2");
while($valuesOut = db2_fetch_array($rs)) {
	var_dump($valuesOut);
}

echo "Values from targ table:\n";
$rs = db2_exec($conn, "SELECT c1, c2 FROM targ ORDER BY c2");
while($valuesOut = db2_fetch_array($rs)) {
	var_dump($valuesOut);
}

db2_close($conn);

?>
--EXPECT--
Transfering data from src table to targ table:
Values from src table:
array(2) {
  [0]=>
  NULL
  [1]=>
  string(26) "1981-05-13 12:56:46.000000"
}
array(2) {
  [0]=>
  int(1)
  [1]=>
  string(26) "1981-05-13 12:56:46.000000"
}
array(2) {
  [0]=>
  int(1)
  [1]=>
  NULL
}
Values from targ table:
array(2) {
  [0]=>
  NULL
  [1]=>
  string(26) "1981-05-13 12:56:46.000000"
}
array(2) {
  [0]=>
  int(1)
  [1]=>
  string(26) "1981-05-13 12:56:46.000000"
}
array(2) {
  [0]=>
  int(1)
  [1]=>
  NULL
}
