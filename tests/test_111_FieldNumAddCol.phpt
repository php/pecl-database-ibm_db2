--TEST--
IBM-DB2: db2_field_num() - add. col. 
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if ($conn) {
    $insert = "INSERT INTO animals values (7, 'cat', 'Benji', 5.1)";
    db2_exec( $conn, $insert );
    
    $stmt = db2_exec( $conn, "SELECT breed, COUNT(breed) AS number FROM animals GROUP BY breed ORDER BY breed" );

    $num1 = db2_field_num( $stmt, "ID" );
    $num2 = db2_field_num( $stmt, "BREED" );
    $num3 = db2_field_num( $stmt, "NUMBER" );
    $num4 = db2_field_num( $stmt, "number" );
    
    $num5 = db2_field_num( $stmt, "Breed" );
    $num6 = db2_field_num( $stmt, 8 );
    $num7 = db2_field_num( $stmt, 1 );
    $num8 = db2_field_num( $stmt, "weight" );
    
    var_dump( $num1 );
    var_dump( $num2 );
    var_dump( $num3 );
    var_dump( $num4 );
    
    var_dump( $num5 );
    var_dump( $num6 );
    var_dump( $num7 );
    var_dump( $num8 );

    db2_rollback($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
bool(false)
int(0)
int(1)
bool(false)
bool(false)
bool(false)
int(1)
bool(false)
