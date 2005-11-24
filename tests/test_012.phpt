--TEST--
IBM-DB2: db2_num_rows - select - scrollable cursor
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $stmt = db2_exec( $conn, "SELECT name FROM animals WHERE weight < 10.0", array('cursor' => DB2_SCROLLABLE) );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: 4
