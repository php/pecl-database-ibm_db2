--TEST--
IBM-DB2: db2_pconnect() - test 500 persistent connections
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

for ($i = 1; $i <= 500; $i++) {
    $pconn[$i] = db2_pconnect($database, $user, $password);
}

if ($pconn[333]) {
    $conn = &$pconn[222];
    require_once('prepare.inc');
    $stmt = db2_exec( $pconn[333], "UPDATE animals SET name = 'flyweight' WHERE weight < 10.0" );
    echo "Number of affected rows: " . db2_num_rows( $stmt );
    db2_close($pconn[333]);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: 4
