--TEST--
IBM-DB2: connect to a database (uncataloged connection)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect("DRIVER={IBM DB2 ODBC DRIVER};DATABASE=$database;HOSTNAME=$hostname;PORT=$port;PROTOCOL=TCPIP;UID=$user;PWD=$password;", '', '');

if ($conn) {
    echo "Connection succeeded.";
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Connection succeeded.
