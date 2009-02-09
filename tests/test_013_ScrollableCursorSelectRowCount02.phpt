--TEST--
IBM-DB2: db2_num_rows - select - scrollable CURSOR
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    $stmt = db2_prepare( $conn, "SELECT * FROM animals WHERE weight < 15.0", array('CURSOR' => DB2_SCROLLABLE) );
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
array(8) {
  ["ID"]=>
  int(0)
  [0]=>
  int(0)
  ["BREED"]=>
  string(3) "cat"
  [1]=>
  string(3) "cat"
  ["NAME"]=>
  string(16) "Pook            "
  [2]=>
  string(16) "Pook            "
  ["WEIGHT"]=>
  string(4) "3.20"
  [3]=>
  string(4) "3.20"
}
