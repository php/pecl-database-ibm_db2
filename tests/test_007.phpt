--TEST--
IBM-DB2: db2_pconnect() - passing options
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$options1 = array('cursor' => DB2_SCROLLABLE);
$options2 = array('cursor' => DB2_FORWARD_ONLY);

$conn = db2_pconnect($database, $user, $password);

if ($conn) {
    $stmt = db2_prepare( $conn, "SELECT name FROM animals WHERE weight < 10.0", $options2 );
    db2_execute($stmt);
    $data = db2_fetch_both($stmt);
    var_dump($data);
    
    echo "\n";

    $stmt = db2_prepare( $conn, "SELECT name FROM animals WHERE weight < 10.0" );
    db2_execute($stmt);
    $data = db2_fetch_both($stmt);
    var_dump($data);

    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
array(2) {
  ["NAME"]=>
  string(16) "Pook            "
  [0]=>
  string(16) "Pook            "
}

array(2) {
  ["NAME"]=>
  string(16) "Pook            "
  [0]=>
  string(16) "Pook            "
}
