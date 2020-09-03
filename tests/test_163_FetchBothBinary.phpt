--TEST--
IBM-DB2: db2_fetch_both() - fetch binary data and iterate over result set
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$in_file = "pic1.jpg";
$in = file_get_contents(dirname(__FILE__)."/".$in_file);

$result = db2_exec($conn, "select photo_format, picture, length(picture) from emp_photo where photo_format='jpg' and empno='000130'");
$row = db2_fetch_both($result);
$out = "";
if ($row) {
    $out .= $row['PICTURE'];
}

if(strcmp($in, $out) == 0) {
    echo "The files are the same...good.";
} else {
    echo "The files are not the same...bad.";
}

$result = db2_exec($conn, "select photo_format, picture, length(picture) from emp_photo");
$count = 0;
while ($row = db2_fetch_both($result)) {
    $count++;
}

echo "\nIterated over $count rows.";
?>
--EXPECT--
The files are the same...good.
Iterated over 8 rows.
