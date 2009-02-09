--TEST--
IBM-DB2: db2_conn_error() - wrong password
--SKIPIF--
<?php
  require_once('skipif.inc');
  require_once('skipif3.inc');
?>
--FILE--
<?php

require_once('connection.inc');

$conn = @db2_connect($database, $user, "z");

if ($conn) {
    echo "??? No way.\n";
}
else {
    $err = db2_conn_error();
    echo $err."\n";
}

?>
--EXPECTF--
0800%d
