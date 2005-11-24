--TEST--
IBM-DB2: db2_rollback() - delete
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_array( $stmt );
    $rows = $res[0];
    echo $rows."\n";
    
    db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );
    $ac = db2_autocommit( $conn );
    if( $ac != 0 ) {
       echo "Cannot set DB2_AUTOCOMMIT_OFF\nCannot run test\n";
       exit;
    }
    
    db2_exec( $conn, "DELETE FROM animals" );
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_array( $stmt );
    $rows = $res[0];
    echo $rows."\n";
    
    db2_rollback( $conn );
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_array( $stmt );
    $rows = $res[0];
    echo $rows."\n";
    db2_close($conn);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
7
0
7
