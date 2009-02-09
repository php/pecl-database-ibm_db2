--TEST--
IBM-DB2: Multiple result sets (uniform column definitions)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$procedure = <<<HERE
  CREATE PROCEDURE multiResults ()
  RESULT SETS 3
  LANGUAGE SQL
  BEGIN
    DECLARE c1 CURSOR WITH RETURN FOR
      SELECT name, id
      FROM animals
      ORDER BY name;

    DECLARE c2 CURSOR WITH RETURN FOR
      SELECT name, id
      FROM animals
      WHERE id < 4
      ORDER BY name DESC;

    DECLARE c3 CURSOR WITH RETURN FOR
      SELECT name, id
      FROM animals
      WHERE weight < 5.0
      ORDER BY name;

    OPEN c1;
    OPEN c2;
    OPEN c3;
  END
HERE;

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if ($conn) {
  @db2_exec($conn, 'DROP PROCEDURE multiResults()');
  db2_exec($conn, $procedure);
  $stmt = db2_exec($conn, 'CALL multiResults()');

  print "Fetching first result set\n";
  while ($row = db2_fetch_array($stmt)) {
    var_dump($row);
  }

  print "Fetching second result set\n";
  $res = db2_next_result($stmt);
  if ($res) {
    while ($row = db2_fetch_array($res)) {
      var_dump($row);
    }
  }

  print "Fetching third result set\n";
  $res2 = db2_next_result($stmt);
  if ($res2) {
    while ($row = db2_fetch_array($res2)) {
      var_dump($row);
    }
  }

  print "Fetching fourth result set (should fail)\n";
  $res3 = db2_next_result($stmt);
  if ($res3) {
    while ($row = db2_fetch_array($res3)) {
      var_dump($row);
    }
  }

  db2_rollback($conn);
  db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Fetching first result set
array(2) {
  [0]=>
  string(16) "Bubbles         "
  [1]=>
  int(3)
}
array(2) {
  [0]=>
  string(16) "Gizmo           "
  [1]=>
  int(4)
}
array(2) {
  [0]=>
  string(16) "Peaches         "
  [1]=>
  int(1)
}
array(2) {
  [0]=>
  string(16) "Pook            "
  [1]=>
  int(0)
}
array(2) {
  [0]=>
  string(16) "Rickety Ride    "
  [1]=>
  int(5)
}
array(2) {
  [0]=>
  string(16) "Smarty          "
  [1]=>
  int(2)
}
array(2) {
  [0]=>
  string(16) "Sweater         "
  [1]=>
  int(6)
}
Fetching second result set
array(2) {
  [0]=>
  string(16) "Smarty          "
  [1]=>
  int(2)
}
array(2) {
  [0]=>
  string(16) "Pook            "
  [1]=>
  int(0)
}
array(2) {
  [0]=>
  string(16) "Peaches         "
  [1]=>
  int(1)
}
array(2) {
  [0]=>
  string(16) "Bubbles         "
  [1]=>
  int(3)
}
Fetching third result set
array(2) {
  [0]=>
  string(16) "Bubbles         "
  [1]=>
  int(3)
}
array(2) {
  [0]=>
  string(16) "Gizmo           "
  [1]=>
  int(4)
}
array(2) {
  [0]=>
  string(16) "Pook            "
  [1]=>
  int(0)
}
Fetching fourth result set (should fail)
