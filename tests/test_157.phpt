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

$conn = db2_connect($db,$username,$password);
if ($conn) {
    $sql = "SELECT id, name, breed, weight FROM animals ORDER BY breed";
    $result = db2_exec($conn, $sql, array('cursor' => DB2_SCROLLABLE));

    $i=2;
    while ($row = db2_fetch_assoc($result, $i)) {
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
