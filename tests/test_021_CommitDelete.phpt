--TEST--
IBM-DB2: db2_commit() - delete
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
    
    db2_autocommit( $conn, 0 );
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
    
    db2_commit( $conn );
    
    $stmt = db2_exec( $conn, "SELECT count(*) FROM animals" );
    $res = db2_fetch_array( $stmt );
    $rows = $res[0];
    echo $rows."\n";

    $animals = array(
      array(0, 'cat',        'Pook',         3.2),
      array(1, 'dog',        'Peaches',      12.3),
      array(2, 'horse',      'Smarty',       350.0),
      array(3, 'gold fish',  'Bubbles',      0.1),
      array(4, 'budgerigar', 'Gizmo',        0.2),
      array(5, 'goat',       'Rickety Ride', 9.7),
      array(6, 'llama',      'Sweater',      150)
    );
    $insert = 'INSERT INTO ANIMALS (ID, BREED, NAME, WEIGHT) VALUES (?, ?, ?, ?)';
    $sth = db2_prepare($conn, $insert);
    if($sth){
       foreach($animals as $row){
          $res = db2_execute($sth, $row);
       }
    }

    db2_commit($conn);
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
