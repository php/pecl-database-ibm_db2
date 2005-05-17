--TEST--
IBM-DB2: db2_bind_param: INSERT statement (one param) - DB2_PARAM_FILE
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

ini_set( "safe_mode", 0 );

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) 
{
    // Drop the test table, in case it exists
    $drop = 'DROP TABLE pictures';
    $result = @db2_exec($conn, $drop);
    
    // Create the test table
    $create = 'CREATE TABLE pictures (id INTEGER, picture BLOB(500K))';
    $result = db2_exec($conn, $create);
    
    $stmt = db2_prepare( $conn, "INSERT INTO pictures VALUES (0, ?)" );
   
    $rc = db2_bind_param( $stmt, 1, "picture", DB2_PARAM_FILE, DB2_BINARY );
    $picture = "./pic1.jpg";

    $rc = db2_execute( $stmt );
    
    $num = db2_num_rows( $stmt );
    
    echo $num;
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
1
