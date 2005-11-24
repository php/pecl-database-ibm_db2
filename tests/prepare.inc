<?php

// Drop the test view, in case it exists
$drop = 'DROP TABLE anime_cat';
$result = @db2_exec($prepconn, $drop);

// Drop the test table, in case it exists
$drop = 'DROP TABLE animals';
$result = @db2_exec($prepconn, $drop);

// Create the test table
$create = 'CREATE TABLE animals (id INTEGER, breed VARCHAR(32), name CHAR(16), weight DECIMAL(7,2))';
$result = db2_exec($prepconn, $create);

// Populate the test table
$animals = array(
    array(0, 'cat', 'Pook', 3.2),
    array(1, 'dog', 'Peaches', 12.3),
    array(2, 'horse', 'Smarty', 350.0),
    array(3, 'gold fish', 'Bubbles', 0.1),
    array(4, 'budgerigar', 'Gizmo', 0.2),
    array(5, 'goat', 'Rickety Ride', 9.7),
    array(6, 'llama', 'Sweater', 150)
);

$insert = 'INSERT INTO animals (id, breed, name, weight) VALUES (?, ?, ?, ?)';
$stmt = db2_prepare($prepconn, $insert);
if ($stmt) {
    foreach ($animals as $animal) {
        $result = db2_execute($stmt, $animal);
    }
}

// Create test view
db2_exec($prepconn, 'CREATE VIEW anime_cat AS
    SELECT name, breed FROM animals
    WHERE id = 0');

?>
