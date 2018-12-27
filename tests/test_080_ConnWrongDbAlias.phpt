--TEST--
IBM-DB2: db2_conn_error() - wrong database alias
--SKIPIF--
<?php require_once('skipif3.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = @db2_connect("x", $user, $password);

if ($conn) {
    echo "??? No way.\n";
}
else {
    echo var_dump(db2_conn_error());
}

?>
--EXPECTF--
string(5) "%s"
