--TEST--
IBM-DB2: db2_pconnect() - passing options
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$options1 = array('cursor' => DB2_SCROLLABLE);
$options2 = array('cursor' => DB2_FORWARD_ONLY);

$conn = db2_pconnect($database, $user, $password, $options1);

if ($conn) {
    $stmt = db2_exec( $conn, "SELECT name FROM animals WHERE weight < 10.0", $options2 );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    
    echo "\n";

    $stmt = db2_exec( $conn, "SELECT name FROM animals WHERE weight < 10.0" );
    echo "Number of affected rows: " . db2_num_rows( $stmt );

    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: -1
Number of affected rows: 4
