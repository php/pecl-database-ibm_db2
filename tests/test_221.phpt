--TEST--
IBM-DB2: db2_pconnect() - test 100 persistent connections
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

for ($i = 1; $i <= 100; $i++) {
    $pconn[$i] = db2_pconnect($database, $user, $password);
}

if ($pconn[33]) {
    $conn = &$pconn[22];
    db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

    $stmt = db2_exec( $pconn[33], "UPDATE animals SET name = 'flyweight' WHERE weight < 10.0" );
    echo "Number of affected rows: " . db2_num_rows( $stmt );

    db2_rollback($conn);
    db2_close($pconn[33]);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: 4
