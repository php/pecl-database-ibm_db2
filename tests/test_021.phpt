--TEST--
IBM-DB2: db2_commit() - delete
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_into( $stmt );
    $rows = $res[0];
    echo $rows."\n";
    
    db2_autocommit( $conn, 0 );
    $ac = db2_autocommit( $conn );
    if( $ac != 0 ) {
       echo "Cannot set DB2_AUTOCOMMIT_OFF\nCannot run test\n";
       exit;
    }
    
    db2_exec( $conn, "DELETE FROM animals" );
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_into( $stmt );
    $rows = $res[0];
    echo $rows."\n";
    
    db2_commit( $conn );
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_into( $stmt );
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
0
