--TEST--
IBM-DB2: db2_tables() - Play with table objects
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$result = db2_tables($conn, NULL, strtoupper($user), 'ANIM%');

while ($row = db2_fetch_object($result)) {
    if (preg_match("/ANIME_CAT|ANIMALS/i",$row->TABLE_NAME)) {
    echo 'Schema:  ' . $row->TABLE_SCHEM . "\n";
    echo 'Name:    ' . $row->TABLE_NAME . "\n";
    echo 'Type:    ' . $row->TABLE_TYPE . "\n";
    echo 'Remarks: ' . $row->REMARKS . "\n\n";
    }
}

db2_free_result($result);
db2_close($conn);

?>
--EXPECTF--
Schema:  %s
Name:    ANIMALS
Type:    TABLE
Remarks: 

Schema:  %s
Name:    ANIME_CAT
Type:    VIEW
Remarks: 

