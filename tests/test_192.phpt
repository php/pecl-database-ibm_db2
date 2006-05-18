--TEST--
IBM-DB2: db2_columns() - 3 - table
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

if ($conn) 
{
   $result = db2_columns($conn,NULL,strtoupper($username),"EMP_RESUME");	
   while ($row = db2_fetch_both($result)) 
   {
      if ($row['COLUMN_NAME'] != 'EMP_ROWID')
      {
         printf ("%s,%s,%s,%s\n", $row[1], 
         $row[2], $row[3], $row[17]);
      }
   }
}
else 
{
   echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECTF--
%s,EMP_RESUME,EMPNO,NO
%s,EMP_RESUME,RESUME_FORMAT,NO
%s,EMP_RESUME,RESUME,YES