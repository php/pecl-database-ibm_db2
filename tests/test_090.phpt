--TEST--
IBM-DB2: db2_conn_errormsg() - wrong database alias
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = @db2_connect("x", $user, $password);

if ($conn) {
    echo "??? No way.\n";
}
else {
    $err = db2_conn_errormsg();
    echo $err."\n";
}

?>
--EXPECT--
[IBM][CLI Driver] SQL1013N  The database alias name or database name "X" could not be found.  SQLSTATE=42705 SQLCODE=-1013
