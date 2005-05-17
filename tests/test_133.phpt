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

if (!$conn) {
    die("Connection failed.\n");
}

require_once('prepare.inc');
$stmt = db2_prepare($conn, $sql);
$res = db2_execute($stmt, array(128, 'hacker of human and technological nature', 'Wez the ruler of all things PECL', 88.3));
var_dump($res);
print "SQLSTATE: " . db2_stmt_error($stmt);
print "\nMessage: " . db2_stmt_errormsg($stmt);

$stmt = db2_prepare($conn, "SELECT breed, name FROM animals WHERE id = ?");
$res = db2_execute ($stmt, array(128));
$row = db2_fetch_assoc($stmt);
print_r($row);

?>
--EXPECTF--
PHP Warning:  db2_execute(): Statement Execute Failed in %stest_133.php on line %d
bool(false)
SQLSTATE: 22001
Message: [IBM][CLI Driver] CLI0109E  String data right truncation. SQLSTATE=22001
