--TEST--
IBM-DB2: db2_stmt_errormsg()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);
if ($conn) {
	$result = @db2_exec($conn,"insert int0 t_string values(123,1.222333,'one to one')");
	if ($result) {
		$cols = db2_num_fields($result);
		echo 'col: ' . $cols . ',';
		$rows = db2_num_rows($result);
		echo 'affected row: ' . $rows ;
	}	
	else {
		echo db2_stmt_errormsg();	
	}
	$result = @db2_exec($conn,"delete from t_string where a=123");
	if ($result) {
		$cols = db2_num_fields($result);
		echo 'col: ' . $cols . ',';
		$rows = db2_num_rows($result);
		echo 'affected row: ' . $rows ;
	}	
	else {
		echo db2_stmt_errormsg();	
	}

}
else {
	echo 'no connection';	
}

?>
--EXPECTF--
Msg:[IBM][CLI Driver][DB2/%s] SQL0104N  An unexpected token "insert int0 t_string" was found following "BEGIN-OF-STATEMENT".  Expected tokens may include:  "<space>".  SQLSTATE=42601
 Err Code: -104col: 0,affected row: 0 
