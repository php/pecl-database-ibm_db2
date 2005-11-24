--TEST--
IBM-DB2: db2_fetch_object() -- access 
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$sql = "SELECT breed, RTRIM(name) AS name
    FROM animals
    WHERE id = ?";

if ($conn) {
    $stmt = db2_prepare($conn, $sql);
    db2_execute($stmt, array(0));
    
    while ($pet = db2_fetch_object($stmt)) {
        echo "Come here, {$pet->NAME}, my little {$pet->BREED}!";
    }
    db2_close($conn);
    
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
Come here, Pook, my little cat!
