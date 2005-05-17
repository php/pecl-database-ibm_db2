--TEST--
IBM-DB2: db2_bind_param: SELECT statement (multiple params)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$sql = "SELECT id, breed, name, weight
    FROM animals
    WHERE weight < ? AND weight > ?";

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_prepare($conn, $sql);

    db2_bind_param($stmt, 1, 'weight', DB2_PARAM_IN);
    db2_bind_param($stmt, 2, 'mass', DB2_PARAM_IN);

    $weight = 200.05;
    $mass = 2.0;

    if (db2_execute($stmt)) {
        while ($row = db2_fetch_array($stmt)) {
            var_dump($row);
        }
    }
    db2_close($conn);
} else {
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
array(4) {
  [0]=>
  int(1)
  [1]=>
  string(3) "dog"
  [2]=>
  string(16) "Peaches         "
  [3]=>
  string(5) "12.30"
}
array(4) {
  [0]=>
  int(5)
  [1]=>
  string(4) "goat"
  [2]=>
  string(16) "Rickety Ride    "
  [3]=>
  string(4) "9.70"
}
array(4) {
  [0]=>
  int(6)
  [1]=>
  string(5) "llama"
  [2]=>
  string(16) "Sweater         "
  [3]=>
  string(6) "150.00"
}
