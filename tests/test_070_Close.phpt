--TEST--
IBM-DB2: db2_close()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    var_dump( $conn );
    
    $rc = db2_close( $conn );
    
    var_dump( $rc );
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECTF--
resource(%d) of type (DB2 Connection)
bool(true)
