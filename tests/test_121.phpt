--TEST--
IBM-DB2: db2_field_name() - add. col.
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
        
    $insert = "INSERT INTO animals values (7, 'cat', 'Benji', 5.1)";
    db2_exec( $conn, $insert );
        
    $stmt = db2_exec( $conn, "SELECT breed, COUNT(breed) AS number FROM animals GROUP BY breed ORDER BY breed" );

    $name1 = db2_field_name( $stmt, 0 );
    $name2 = db2_field_name( $stmt, 1 );
    $name3 = db2_field_name( $stmt, 2 );
    $name4 = db2_field_name( $stmt, 3 );
    
    $name5 = db2_field_name( $stmt, "BREED" );
    $name6 = db2_field_name( $stmt, 7 );
    $name7 = db2_field_name( $stmt, '"Number"' );
    $name8 = db2_field_name( $stmt, "NUMBER" );
    
    var_dump( $name1 );
    var_dump( $name2 );
    var_dump( $name3 );
    var_dump( $name4 );
    
    var_dump( $name5 );
    var_dump( $name6 );
    var_dump( $name7 );
    var_dump( $name8 );
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
string(5) "BREED"
string(6) "NUMBER"
bool(false)
bool(false)
string(5) "BREED"
bool(false)
bool(false)
string(6) "NUMBER"
