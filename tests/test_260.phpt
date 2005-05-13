--TEST--
IBM-DB2: db2_fetch_object()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_exec( $conn, "SELECT id, breed, name, weight FROM animals WHERE id = 0" );
    
    while ($object = db2_fetch_object($stmt)) {
        var_dump( $object );
    }
    db2_close($conn);
    
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
object(stdClass)#1 (4) {
  ["ID"]=>
  int(0)
  ["BREED"]=>
  string(3) "cat"
  ["NAME"]=>
  string(16) "Pook            "
  ["WEIGHT"]=>
  string(4) "3.20"
}
