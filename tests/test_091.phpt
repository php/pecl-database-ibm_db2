--TEST--
IBM-DB2: db2_conn_errormsg() - wrong username
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = @db2_connect($database, "y", $password);

if ($conn) {
    echo "??? No way.\n";
}
else {
    $err = db2_conn_errormsg();
    echo $err."\n";
}

?>
--EXPECT--
Msg:[IBM][CLI Driver] SQL30082N  Attempt to establish connection failed with security reason "24" ("USERNAME AND/OR PASSWORD INVALID").  SQLSTATE=08001
 Err Code: -30082
