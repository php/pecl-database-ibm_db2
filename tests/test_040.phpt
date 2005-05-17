--TEST--
IBM-DB2: db2_fetch_array one row
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

// Drop the test table, in case it exists
$drop = 'DROP TABLE animals';
$result = @db2_exec($conn, $drop);

// Create the test table
$create = 'CREATE TABLE animals (id INTEGER, breed VARCHAR(32), name CHAR(16), weight DECIMAL(7,2))';
$result = db2_exec($conn, $create);

$insert = "INSERT INTO animals values (0, 'cat', 'Pook', 3.2)";

db2_exec( $conn, $insert );

$stmt = db2_exec( $conn, "select * from animals" );

$onerow = db2_fetch_array( $stmt );

var_dump( $onerow );

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
