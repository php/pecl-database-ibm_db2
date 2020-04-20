--TEST--
IBM-DB2: db2_execute with excessively long input parameters
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$sql =  "INSERT INTO animals (id, breed, name, weight)
    VALUES (?, ?, ?, ?)";

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if (!$conn) {
    die("Connection failed.\n");
}

$stmt = db2_prepare($conn, $sql);
$res = @db2_execute($stmt, array(128, 'hacker of human and technological nature', 'Wez the ruler of all things PECL', 88.3));
var_dump($res);
print "SQLSTATE: " . db2_stmt_error($stmt);
print "\nMessage: " . db2_stmt_errormsg($stmt);

$stmt = db2_prepare($conn, "SELECT breed, name FROM animals WHERE id = ?");
$res = db2_execute ($stmt, array(128));
$row = db2_fetch_assoc($stmt);
print_r($row);

db2_rollback($conn);

?>
--EXPECTF--
bool(false)
SQLSTATE: 22001
Message: Value for column or variable NAME too long.
