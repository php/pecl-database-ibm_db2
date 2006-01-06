--TEST--
IBM-DB2: retrieve CLOB columns: scrollable cursor
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$sql = "SELECT empno, resume_format, resume
FROM emp_resume
WHERE resume_format = 'ascii'";

if (!$conn) {
   die('no connection: ' . db2_conn_errormsg());	
}
$result = db2_exec($conn, $sql, array('cursor' => DB2_SCROLLABLE));
$i = 1;
while ($row = db2_fetch_array($result, $i++)) {
  print_r($row);
}
?>
--EXPECTREGEX--
(PHP )?Warning:\s*db2_fetch_array\(\): Fetch Failure in .*?test_194.php on line 16
