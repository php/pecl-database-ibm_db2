--TEST--
IBM-DB2: db2_conn_errormsg() - wrong username
--SKIPIF--
<?php
  require_once('skipif.inc');
  require_once('skipif3.inc');
?>
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
--EXPECTF--
Authorization failure on distributed database connection attempt. SQLCODE=-30082
