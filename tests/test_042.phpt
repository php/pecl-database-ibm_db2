--TEST--
IBM-DB2: db2_fetch_into() - several rows 1
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);
//$_GET['EMPNO'] = '000130';

if (isset($_GET['EMPNO'])) {
	$result = db2_exec($conn, "select photo_format, picture, length(picture) from emp_photo where photo_format='gif' and empno='".$_GET['EMPNO']."'");
	$row = db2_fetch_into($result); 			
	if ($row) {
		// We'll be outputting a 		
		header('Content-type: image/'. $row[0]);
		header('Content-Length: '. $row[2]);
		echo $row[1];			
	}
	else {
		echo $db2_error();			
	}
	exit();
}
else {
	$result = db2_exec($conn, "select EMPNO, PHOTO_FORMAT from emp_photo where photo_format='gif'");	
	while ($row = db2_fetch_into($result)) {
		printf ("<a href='test_042.php?EMPNO=%s' target=_blank>%s (%s)</a><br>",$row['0'], $row['0'], $row[1]);
		print "\n";
	}
}


?>
--EXPECT--
<a href='test_042.php?EMPNO=000130' target=_blank>000130 (gif)</a><br>
<a href='test_042.php?EMPNO=000140' target=_blank>000140 (gif)</a><br>
<a href='test_042.php?EMPNO=000150' target=_blank>000150 (gif)</a><br>
<a href='test_042.php?EMPNO=000190' target=_blank>000190 (gif)</a><br>

