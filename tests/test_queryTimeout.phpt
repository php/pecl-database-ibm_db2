--TEST--
IBM-DB2: Query Time Out test.
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if($conn) {
	echo "Connection pass.\n";
	$createQuery = "CREATE TABLE querytimeout(id INTEGER, value INTEGER)";
	$dropQuery = "DROP TABLE querytimeout";

	$rc = @db2_exec ($conn, $dropQuery);
	$rc = db2_exec ($conn, $createQuery);

	$insertQuery = "INSERT INTO querytimeout (id, value) VALUES(?, ?)";

	$stmt = db2_prepare ($conn, $insertQuery);

	db2_set_option($conn, array("query_timeout" => "500"), 1);

	for($i = 0;$i < 10;$i++) {
		$rc = db2_execute($stmt, array($i, $i + $i));
	}
	
	$selectQuery = "SELECT * FROM querytimeout";

	$stmt = db2_prepare($conn, $selectQuery);

	db2_set_option($stmt, array("query_timeout" => 1), 2);

	db2_execute($stmt);

	while ($row = db2_fetch_array($stmt)) {
		print_r($row);
	}

	db2_close($conn);
} else {
	echo "Connection fail.\n";
	echo db2_conn_errormsg() . "\n";
}
?>
--EXPECTF--
Connection pass.

Warning: db2_set_option(): QUERY TIMEOUT attribute can only be set on statement resources in %s on line 18
Array
(
    [0] => 0
    [1] => 0
)
Array
(
    [0] => 1
    [1] => 2
)
Array
(
    [0] => 2
    [1] => 4
)
Array
(
    [0] => 3
    [1] => 6
)
Array
(
    [0] => 4
    [1] => 8
)
Array
(
    [0] => 5
    [1] => 10
)
Array
(
    [0] => 6
    [1] => 12
)
Array
(
    [0] => 7
    [1] => 14
)
Array
(
    [0] => 8
    [1] => 16
)
Array
(
    [0] => 9
    [1] => 18
)
