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
	@$stmt = db2_column_privileges($conn, NULL, NULL, 'ANIMALS');
	$row = db2_fetch_array($stmt);
	print $row[2] . "\n";
	print $row[3] . "\n";
	print $row[7];
    db2_close($conn);
}
else
{
	echo db2_conn_errormsg();
	printf("Connection failed\n\n");
}

?>
--EXPECTF--
ANIMALS
BREED
YES
