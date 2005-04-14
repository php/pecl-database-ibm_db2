--TEST--
IBM-DB2: db2_bind_param: INSERT statement - NULL parameter
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

$insert = "INSERT INTO animals (id, breed, name, weight)
    VALUES (NULL, 'ghost', NULL, NULL)";
$select = 'SELECT id, breed, name, weight
    FROM animals WHERE weight IS NULL';

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_prepare( $conn, $insert);

    if (db2_execute($stmt)) {
        $stmt = db2_exec($conn, $select);
	while ($row = db2_fetch_into($stmt)) {
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
  NULL
  [1]=>
  string(5) "ghost"
  [2]=>
  NULL
  [3]=>
  NULL
}
