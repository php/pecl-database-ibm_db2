--TEST--
IBM-DB2: db2_field_name() - colnames diff. case
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $drop = "drop table ftest";
    @db2_exec( $conn, $drop );
    
    $create = "create table ftest ( TEST integer, \"test\" integer, \"Test\" integer  )";
    db2_exec( $conn, $create );
    
    $insert = "INSERT INTO ftest values (1,2,3)";
    db2_exec( $conn, $insert );
    
    $stmt = db2_exec( $conn, "SELECT * FROM ftest" );

    $num1 = db2_field_name( $stmt, 0 );
    $num2 = db2_field_name( $stmt, 1 );
    $num3 = db2_field_name( $stmt, 2 );
    
    $num4 = db2_field_name( $stmt, "TEST" );
    $num5 = db2_field_name( $stmt, '"test"' );
    $num6 = db2_field_name( $stmt, '"Test"' );
        
    var_dump( $num1 );
    var_dump( $num2 );
    var_dump( $num3 );
    
    var_dump( $num4 );
    var_dump( $num5 );
    var_dump( $num6 );
   
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
string(4) "TEST"
string(4) "test"
string(4) "Test"
string(4) "TEST"
string(4) "test"
string(4) "Test"
