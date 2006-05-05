--TEST--
IBM-DB2: db2_conn_errormsg() - wrong password
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = @db2_connect($database, $user, "z");

if ($conn) {
    echo "??? No way.\n";
}
else {
    $err = db2_conn_errormsg();
    echo $err."\n";
}

?>
--EXPECTF--
[IBM][CLI Driver] SQL30082N %s reason "24" ("USERNAME AND/OR PASSWORD INVALID").  SQLSTATE=08001 SQLCODE=-30082
