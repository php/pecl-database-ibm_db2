--TEST--
IBM-DB2: db2_num_rows - select (rowcount prefetch) - prep option
--SKIPIF--
<?php require_once('skipifNotRowcountPrefetch.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_ON );

if ($conn) {
    $options = array('rowcount' => DB2_ROWCOUNT_PREFETCH_ON);
    $stmt = db2_prepare( $conn, "SELECT * from animals WHERE weight < 10.0", $options );
    db2_execute( $stmt );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: 4
