--TEST--
IBM-DB2: db2_result -- request a column that does not exist
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_exec( $conn, "SELECT id, breed, name, weight FROM animals WHERE id = 0" );
    
    while (db2_fetch_row($stmt) == TRUE) {
        $breed = db2_result( $stmt, 1 );
        var_dump( $breed );
        $name = db2_result( $stmt, "NAME" );
        var_dump( $name );

        // following field does not exist in result set
        @$name = db2_result( $stmt, "PASSPORT" );
        var_dump( $name );
    }
    db2_close($conn);
    
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
string(3) "cat"
string(16) "Pook            "
NULL
