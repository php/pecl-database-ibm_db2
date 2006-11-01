--TEST--
IBM-DB2: PECL bug 6561 -- returning NULL values from column functions
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if ($conn) {
    $stmt = db2_exec( $conn, "INSERT INTO animals (id, breed, name, weight) VALUES (null, null, null, null)");
    $statement = "SELECT count(id) FROM animals"; 
    $result = db2_exec($conn, $statement);
    if (!$result && db2_stmt_error()) { 
        printf("ERROR: %s\n", db2_stmt_errormsg()); 
    } 
    while ($row = db2_fetch_array($result)) { 
        var_dump($row);
    }

    db2_rollback($conn);
    db2_close($conn);
    
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
array(1) {
  [0]=>
  int(7)
}
