--TEST--
IBM-DB2: db2_fetch_array() - fetch one row of binary data
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$orig = fopen("spook.png", "rb");
$new = fopen("spook_out.png", "wb");

$result = db2_exec($conn, "SELECT picture, LENGTH(picture) FROM animal_pics WHERE name = 'Spook'");
$row = db2_fetch_array($result);
if ($row) {
    fwrite($new, $row[0]);
}

$file0 = file_get_contents("spook.png");
$file1 = file_get_contents("spook_out.png");

if(strcmp($file0, $file1) == 0) {
    echo "The files are the same...good.";
} else {
    echo "The files are not the same...bad.";
}
?>
--EXPECT--
The files are the same...good.
