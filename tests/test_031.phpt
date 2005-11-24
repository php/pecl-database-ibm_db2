--TEST--
IBM-DB2: db2_result (index by position)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $stmt = db2_exec( $conn, "SELECT id, breed, name, weight FROM animals WHERE id = 0" );
    
    while (db2_fetch_row($stmt) == TRUE) {
        $id = db2_result( $stmt, 0 );
        var_dump( $id );
        $breed = db2_result( $stmt, 1 );
        var_dump( $breed );
        $name = db2_result( $stmt, 2 );
        var_dump( $name );
        $weight = db2_result( $stmt, 3 );
        var_dump( $weight );
    }
    db2_close($conn);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
int(0)
string(3) "cat"
string(16) "Pook            "
string(4) "3.20"
