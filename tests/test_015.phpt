--TEST--
IBM-DB2: db2_num_rows - insert, delete
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);
if ($conn) {
	$result = db2_exec($conn,"insert into t_string values(123,1.222333,'one to one')");
	if ($result) {
		$cols = db2_num_fields($result);
		echo 'col: ' . $cols . ',';
		$rows = db2_num_rows($result);
		echo 'affected row: ' . $rows ;
	}	
	else {
		echo db2_stmt_errormsg();	
	}
	$result = db2_exec($conn,"delete from t_string where a=123");
	if ($result) {
		$cols = db2_num_fields($result);
		echo 'col: ' . $cols . ',';
		$rows = db2_num_rows($result);
		echo 'affected row: ' . $rows ;
	}	
	else {
		echo db2_stmt_errormsg();	
	}
	db2_close($conn);
}
else {
	echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECT--
col: 0,affected row: 1col: 0,affected row: 1 
