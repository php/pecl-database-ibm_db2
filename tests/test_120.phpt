--TEST--
IBM-DB2: db2_field_name()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_exec( $conn, "SELECT * FROM animals" );

    $name1 = db2_field_name( $stmt, 1 );
    $name2 = db2_field_name( $stmt, 2 );
    $name3 = db2_field_name( $stmt, 3 );
    $name4 = db2_field_name( $stmt, 4 );
    
    $name5 = db2_field_name( $stmt, "ID" );
    $name6 = db2_field_name( $stmt, 8 );
    $name7 = db2_field_name( $stmt, 0 );
    $name8 = db2_field_name( $stmt, "weight" );
    
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
string(4) "NAME"
string(6) "WEIGHT"
bool(false)
string(2) "ID"
bool(false)
string(2) "ID"
string(6) "WEIGHT"
