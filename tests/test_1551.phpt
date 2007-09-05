--TEST--
IBM-DB2: db2_fetch_assoc() - fetch binary data and iterate over result set
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$orig = fopen("pic1.jpg", "rb");
$new = fopen("pic1_out.jpg", "wb");

$result = db2_exec($conn, "select photo_format, picture, length(picture) from emp_photo where photo_format='jpg' and empno='000130'");
$row = db2_fetch_assoc($result);
if ($row) {
    fwrite($new, $row['PICTURE']);
}

$file0 = file_get_contents("pic1.jpg");
$file1 = file_get_contents("pic1_out.jpg");

if(strcmp($file0, $file1) == 0) {
    echo "The files are the same...good.";
} else {
    echo "The files are not the same...bad.";
}

$result = db2_exec($conn, "select photo_format, picture, length(picture) from emp_photo");
$count = 0;
while ($row = db2_fetch_assoc($result)) {
    $count++;
}

echo "\nIterated over $count rows.";
--EXPECT--
The files are the same...good.
Iterated over 8 rows.
