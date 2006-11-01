--TEST--
IBM-DB2: db2_field_num() - colnames diff. case
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

    $num1 = db2_field_num( $stmt, "TEST" );
    $num2 = db2_field_num( $stmt, 'test' );
    $num3 = db2_field_num( $stmt, 'Test' );
        
    var_dump( $num1 );
    var_dump( $num2 );
    var_dump( $num3 );

    $drop = "drop table ftest";
    @db2_exec( $conn, $drop );
    
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
int(0)
int(1)
int(2)
