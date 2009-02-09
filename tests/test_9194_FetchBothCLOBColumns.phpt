--TEST--
IBM-DB2: PECL bug 9194 -- db2_fetch_both doesn't work on null CLOB columns
--SKIPIF--
<?php
  require_once('skipif.inc');
?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
	$sql = "DROP TABLE test_9194";
	@db2_exec($conn, $sql);

	$sql = "CREATE TABLE test_9194 (id INT, myclob CLOB)";
	db2_exec($conn, $sql);

	$sql = "INSERT INTO test_9194 (id, myclob) VALUES (1, null)";
	db2_exec($conn, $sql);

	$sql = "SELECT * FROM test_9194";
	$stmt = db2_prepare($conn, $sql);
	db2_execute($stmt);
	$row = db2_fetch_both($stmt);
	var_dump($row);
}
else {
	echo "Connection failed.\n";
}

?>
--EXPECT--
array(4) {
  ["ID"]=>
  int(1)
  [0]=>
  int(1)
  ["MYCLOB"]=>
  NULL
  [1]=>
  NULL
}
