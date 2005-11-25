<?php

// Drop the stored procedure, in case it exists
$drop = 'DROP PROCEDURE match_animal';
$result = @db2_exec($conn, $drop);

// Create the stored procedure
$result = db2_exec($conn, "
CREATE PROCEDURE match_animal(IN first_name VARCHAR(128), INOUT second_name VARCHAR(128), OUT animal_weight DOUBLE)
DYNAMIC RESULT SETS 1
MODIFIES SQL DATA
LANGUAGE SQL
NO EXTERNAL ACTION
BEGIN
  DECLARE match_name INT DEFAULT 0;
  DECLARE c1 CURSOR FOR
    SELECT COUNT(*) FROM animals
    WHERE name IN (second_name);

  DECLARE c2 CURSOR FOR
    SELECT SUM(weight) FROM animals
    WHERE name in (first_name, second_name);
  
  DECLARE c3 CURSOR WITH RETURN FOR 
    SELECT name, breed, weight FROM animals
    WHERE name BETWEEN first_name AND second_name
    ORDER BY name;

  OPEN c1;
  FETCH c1 INTO match_name;
  IF (match_name > 0)
    THEN SET second_name = 'TRUE';
  END IF;
  CLOSE c1;

  OPEN c2;
  FETCH c2 INTO animal_weight;
  CLOSE c2;

  OPEN c3;
END
");

$result = null;

?>
