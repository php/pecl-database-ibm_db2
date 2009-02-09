--TEST--
IBM-DB2: Bind and retrieve NULL and empty string values
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if ($conn) {
    $stmt = db2_prepare( $conn, "INSERT INTO animals (id, breed, name) VALUES (?, ?, ?)" );

    $id = 999;
    $breed = NULL;
    $name = '';
    db2_bind_param($stmt, 1, 'id');
    db2_bind_param($stmt, 2, 'breed');
    db2_bind_param($stmt, 3, 'name');

    /* 
       After this statement, we expect that the BREED column will contain
       an SQL NULL value, while the NAME column contains an empty string
    */
    db2_execute($stmt); 

    /* 
       After this statement, we expect that the BREED column will contain
       an SQL NULL value, while the NAME column contains an empty string.
       Use the dynamically bound parameters to ensure that the code paths
       for both db2_bind_param and db2_execute treat PHP nulls and empty
       strings the right way.
    */
    db2_execute($stmt, array(1000, NULL, '')); 

    $result = db2_exec($conn, "SELECT id, breed, name FROM animals WHERE breed IS NULL");
    while ($row = db2_fetch_array($result)) {
      var_dump($row);
    }

    db2_rollback($conn);
}
else 
{
    echo "Connection failed.\n";
}

?>
--EXPECT--
array(3) {
  [0]=>
  int(999)
  [1]=>
  NULL
  [2]=>
  string(16) "                "
}
array(3) {
  [0]=>
  int(1000)
  [1]=>
  NULL
  [2]=>
  string(16) "                "
}
