--TEST--
IBM-DB2: db2_columns() - 2 - table
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

if ($conn) {
	$result = db2_columns($conn,NULL,NULL,"EMP_PHOTO");	
	$i = 0;
	while ($row = db2_fetch_both($result)) {
		if ($row['COLUMN_NAME'] != 'EMP_ROWID' && $i < 3) {
			printf ("%s,%s,%s,%s\n", $row['TABLE_SCHEM'], 
				$row['TABLE_NAME'], $row['COLUMN_NAME'], $row['IS_NULLABLE']);
		}
		$i++;
	}
	print "done!";
} else {
	echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECTF--
%s,EMP_PHOTO,EMPNO,NO
%s,EMP_PHOTO,PHOTO_FORMAT,NO
%s,EMP_PHOTO,PICTURE,YES
done! 
