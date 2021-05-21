--TEST--
IBM-DB2: db2_num_fields() - select
--SKIPIF--
<?php
  require_once('skipif.inc');
?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db, $username, $password);
if (!$conn) {
   echo db2_conn_errormsg();
}

$result = db2_exec($conn, "VALUES(1)");
print db2_num_fields($result);
db2_close($conn);

?>
--EXPECT--
1
