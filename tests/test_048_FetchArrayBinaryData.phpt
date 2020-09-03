--TEST--
IBM-DB2: db2_fetch_array() - fetch one row of binary data
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$in_file = "spook.png";
$in = file_get_contents(dirname(__FILE__)."/".$in_file);

$result = db2_exec($conn, "SELECT picture, LENGTH(picture) FROM animal_pics WHERE name = 'Spook'");
$row = db2_fetch_array($result);
$out = "";
if ($row) {
    $out .= $row[0];
}

if(strcmp($in, $out) == 0) {
    echo "The files are the same...good.";
} else {
    echo "The files are not the same...bad.";
}
?>
--EXPECT--
The files are the same...good.
