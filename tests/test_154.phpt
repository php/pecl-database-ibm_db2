--TEST--
IBM-DB2: db2_fetch_assoc() - simple select 5
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$result = db2_exec($conn, "select * from in_tray");

while ($row = db2_fetch_assoc($result)) {
 
 
 
 
printf ("%26s ",$row['RECEIVED']);
printf ("%-8s ",$row['SOURCE']);
printf ("%-64s ",$row['SUBJECT']);
print "\n";
printf ($row['NOTE_TEXT']);
print "\n-----------\n";
}

?>
--EXPECT--

