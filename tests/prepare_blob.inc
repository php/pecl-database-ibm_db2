<?php

// Drop the test table, in case it exists
$drop = 'DROP TABLE animal_pics';
$result = @db2_exec($conn, $drop);

// Create the test table
$create = 'CREATE TABLE animal_pics (name VARCHAR(32), picture BLOB(16K))';
$result = db2_exec($conn, $create);

// Populate the test table
$animals = array(
    array('Spook', 'spook.png'),
    array('Helmut', 'pic1.jpg'),
);

// Insert the pictures
$insert = 'INSERT INTO animal_pics (name, picture) VALUES (?, ?)';
$stmt = db2_prepare($conn, $insert);
if (!$stmt) {
  die("Attempt to prepare statement failed.");
}
foreach ($animals as $animal) {
    $name = $animal[0];
    $picture = file_get_contents($animal[1]);
    if (!$picture) {
      die("Could not retrieve picture '{$animal[1]}'.");
    }
    db2_bind_param($stmt, 1, 'name', DB2_PARAM_IN);
    db2_bind_param($stmt, 2, 'picture', DB2_PARAM_IN, DB2_BINARY);
    $result = db2_execute($stmt);
}

?>
