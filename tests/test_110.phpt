--TEST--
IBM-DB2: db2_field_num()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_exec( $conn, "SELECT * FROM animals ORDER BY breed" );

    $num1 = db2_field_num( $stmt, "ID" );
    $num2 = db2_field_num( $stmt, "BREED" );
    $num3 = db2_field_num( $stmt, "NAME" );
    $num4 = db2_field_num( $stmt, "WEIGHT" );
    
    $num5 = db2_field_num( $stmt, "TEST" );
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
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
int(0)
int(1)
int(2)
int(3)
bool(false)
bool(false)
int(1)
int(3)
