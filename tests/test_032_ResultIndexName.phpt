--TEST--
IBM-DB2: db2_result (index by name)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $stmt = db2_exec( $conn, "SELECT id, breed, name, weight FROM animals WHERE id = 6" );
    
    while (db2_fetch_row($stmt) == TRUE) {
        $id = db2_result( $stmt, "ID" );
        var_dump( $id );
        $breed = db2_result( $stmt, "BREED" );
        var_dump( $breed );
        $name = db2_result( $stmt, "NAME" );
        var_dump( $name );
        $weight = db2_result( $stmt, "WEIGHT" );
        var_dump( $weight );
    }
    db2_close($conn);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
int(6)
string(5) "llama"
string(16) "Sweater         "
string(6) "150.00"
