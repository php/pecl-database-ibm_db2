--TEST--
IBM-DB2: db2_num_fields() - select - delete - insert
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $stmt = db2_exec( $conn, "SELECT * FROM animals ORDER BY breed" );

    $fields1 = db2_num_fields( $stmt );
    
    var_dump( $fields1 );
    
    $stmt = db2_exec( $conn, "SELECT name, breed FROM animals ORDER BY breed" );
    $fields2 = db2_num_fields( $stmt );
    
    var_dump( $fields2 );
    
    $stmt = db2_exec( $conn, "DELETE FROM animals" );
    $fields3 = db2_num_fields( $stmt );
    
    var_dump( $fields3 );
    
    $stmt = db2_exec( $conn, "INSERT INTO animals values (0, 'cat', 'Pook', 3.2)" );
    $fields4 = db2_num_fields( $stmt );
        
    var_dump( $fields4 );
    
    $stmt = db2_exec( $conn, "SELECT name, breed, 'TEST' FROM animals" );
    $fields5 = db2_num_fields( $stmt );
        
    var_dump( $fields5 );
}    
else {
    echo "Connection failed.";
}

?>
--EXPECT--
int(4)
int(2)
int(0)
int(0)
int(3)
