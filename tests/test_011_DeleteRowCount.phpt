--TEST--
IBM-DB2: db2_num_rows - delete
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if ($conn) {
    $stmt = db2_exec( $conn, "DELETE FROM animals WHERE weight > 10.0" );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    db2_rollback($conn);
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: 3
