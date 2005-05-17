--TEST--
IBM-DB2: db2_execute statement with array of multiple parameters
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$sql =  "SELECT id, breed, name, weight
    FROM animals
    WHERE id = ? AND name = ?";

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_prepare( $conn, $sql);

    if (db2_execute($stmt, array(0, 'Pook'))) {
        while ($row = db2_fetch_array($stmt)) {
            var_dump($row);
        }
    }
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
array(4) {
  [0]=>
  int(0)
  [1]=>
  string(3) "cat"
  [2]=>
  string(16) "Pook            "
  [3]=>
  string(4) "3.20"
}
