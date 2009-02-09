--TEST--
IBM-DB2: db2_num_rows - select (rowcount prefetch) - set option
--SKIPIF--
<?php require_once('skipifNotRowcountPrefetch.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_ON );

if ($conn) {
    $stmt = db2_prepare( $conn, "SELECT * from animals WHERE weight < 10.0" );
    $options = array('rowcount' => DB2_ROWCOUNT_PREFETCH_ON);
    db2_set_option($stmt, $options, 2);
    db2_execute( $stmt );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    echo "\n";
    db2_free_result( $stmt );
    $options = array('rowcount' => DB2_ROWCOUNT_PREFETCH_OFF);
    db2_set_option($stmt, $options, 2);
    db2_execute( $stmt );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    echo "\n";
    db2_free_result( $stmt );
    $options = array('rowcount' => DB2_ROWCOUNT_PREFETCH_ON);
    db2_set_option($stmt, $options, 2);
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
Number of affected rows: -1
Number of affected rows: 4
