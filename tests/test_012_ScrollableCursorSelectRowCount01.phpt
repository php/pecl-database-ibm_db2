--TEST--
IBM-DB2: db2_num_rows - select - scrollable cursor
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $stmt = db2_prepare( $conn, "SELECT name FROM animals WHERE weight < 10.0", array('cursor' => DB2_SCROLLABLE) );
    db2_execute($stmt);
    $row = db2_fetch_both($stmt);
    var_dump($row);
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
array(2) {
  ["NAME"]=>
  string(16) "Pook            "
  [0]=>
  string(16) "Pook            "
}
