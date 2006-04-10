--TEST--
IBM-DB2: db2_column_privileges -- tests the db2_column_privileges functionality
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn != 0)
{
	$stmt = db2_column_privileges($conn, NULL, NULL, 'ANIMALS');
	$row = db2_fetch_array($stmt);
	var_dump($row);
    db2_close($conn);
}
else
{
	echo db2_conn_errormsg();
	printf("Connection failed\n\n");
}

?>
--EXPECT--
array(8) {
  [0]=>
  NULL
  [1]=>
  string(8) "INFORMIX"
  [2]=>
  string(7) "ANIMALS"
  [3]=>
  string(5) "BREED"
  [4]=>
  string(6) "SYSIBM"
  [5]=>
  string(8) "INFORMIX"
  [6]=>
  string(6) "INSERT"
  [7]=>
  string(3) "YES"
}
