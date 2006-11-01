--TEST--
IBM-DB2: db2_fetch_array() - several rows 4
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);
//$_GET['EMPNO'] = '000130';

if (isset($_GET['EMPNO']) && isset($_GET['FORMAT'])) {
	$result = db2_exec($conn, "select photo_format, picture, length(picture) from emp_photo where photo_format='" . $_GET['FORMAT'] . "' and empno='".$_GET['EMPNO']."'");
	$row = db2_fetch_array($result); 			
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
	$result = db2_exec($conn, "select EMPNO, PHOTO_FORMAT, length(PICTURE) from emp_photo");	
	while ($row = db2_fetch_array($result)) {
		if( $row[1] != 'xwd' ) {
			printf ("<a href='test_046.php?EMPNO=%s&FORMAT=%s' target=_blank>%s - %s - %s bytes</a><br>",$row['0'], $row[1], $row['0'], $row[1], $row[2]);
			print "\n";
		}
	}
}

?>
--EXPECT--
<a href='test_046.php?EMPNO=000130&FORMAT=jpg' target=_blank>000130 - jpg - 15398 bytes</a><br>
<a href='test_046.php?EMPNO=000130&FORMAT=png' target=_blank>000130 - png - 10291 bytes</a><br>
<a href='test_046.php?EMPNO=000140&FORMAT=jpg' target=_blank>000140 - jpg - 15398 bytes</a><br>
<a href='test_046.php?EMPNO=000140&FORMAT=png' target=_blank>000140 - png - 10291 bytes</a><br>
<a href='test_046.php?EMPNO=000150&FORMAT=jpg' target=_blank>000150 - jpg - 15398 bytes</a><br>
<a href='test_046.php?EMPNO=000150&FORMAT=png' target=_blank>000150 - png - 10291 bytes</a><br>
<a href='test_046.php?EMPNO=000190&FORMAT=jpg' target=_blank>000190 - jpg - 15398 bytes</a><br>
<a href='test_046.php?EMPNO=000190&FORMAT=png' target=_blank>000190 - png - 10291 bytes</a><br>

