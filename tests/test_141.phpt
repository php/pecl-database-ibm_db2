--TEST--
IBM-DB2: db2_bind_param: SELECT statement (multiple params)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$sql = "SELECT id, breed, name, weight
    FROM animals
    WHERE id < ? AND weight > ?";

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_prepare( $conn, $sql );

    $animal = 5;
    $mass = 2.0;
    db2_bind_param($stmt, 1, 'animal');
    db2_bind_param($stmt, 2, 'mass');

    if (db2_execute($stmt)) 
    {
        while ($row = db2_fetch_into($stmt)) {
            var_dump($row);
        }
    }
    db2_close($conn);
}
else 
{
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
  int(2)
  [1]=>
  string(5) "horse"
  [2]=>
  string(16) "Smarty          "
  [3]=>
  string(6) "350.00"
}
