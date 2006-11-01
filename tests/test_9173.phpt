--TEST--
IBM-DB2: PECL bug 9173 -- db2_procedures & db2_ procedure_columns doesn't work on persistent connections
--SKIPIF--
<?php
  require_once('skipif.inc');
?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_pconnect($database, $user, $password);

if ($conn) {
	$stmt = db2_procedures($conn, NULL, 'SYS%', '%%');

	$row = db2_fetch_assoc($stmt);
	var_dump($row);
}
else {
	echo "Connection failed.\n";
}

?>
--EXPECTF--
array(8) {
  ["PROCEDURE_CAT"]=>
  %s
  ["PROCEDURE_SCHEM"]=>
  string(%d) "%s"
  ["PROCEDURE_NAME"]=>
  string(%d) "%s"
  ["NUM_INPUT_PARAMS"]=>
  int(%d)
  ["NUM_OUTPUT_PARAMS"]=>
  int(%d)
  ["NUM_RESULT_SETS"]=>
  int(%d)
  ["REMARKS"]=>
  NULL
  ["PROCEDURE_TYPE"]=>
  int(%d)
}
