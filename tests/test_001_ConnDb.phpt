--TEST--
IBM-DB2: connect to a database
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
    echo "Connection succeeded.";
    db2_close($conn);
}
else {
    echo "Connection failed: " . db2_conn_errormsg();
}

?>
--EXPECT--
Connection succeeded.
