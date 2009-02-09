--TEST--
IBM-DB2: db2_fetch_assoc() - positioned with scrollable cursor
--SKIPIF--
<?php
  require_once('skipif.inc');
  require_once('skipif3.inc');
?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
if ($conn) {
    $sql = "SELECT id, name, breed, weight FROM animals ORDER BY breed";
    $stmt = db2_prepare($conn, $sql, array('cursor' => DB2_SCROLLABLE));
    $result = db2_execute($stmt);

    $i=2;
    while ($row = db2_fetch_assoc($stmt, $i)) {
        printf ("%-5d %-16s %-32s %10s\n", 
            $row['ID'], $row['NAME'], $row['BREED'], $row['WEIGHT']);
    
        $i = $i + 2;
    }
}

?>
--EXPECT--
0     Pook             cat                                    3.20
5     Rickety Ride     goat                                   9.70
2     Smarty           horse                                350.00
