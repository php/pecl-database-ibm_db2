--TEST--
IBM-DB2: db2_close() - success
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db, $username, $password);

if ($conn) 
{
   $rc = db2_close($conn);
   if ( $rc == true )
      print "db2_close succeeded\n";
   else
      print "db2_close FAILED\n";
}
else 
{
   echo db2_conn_errormsg();	
   echo ',sqlstate=' . db2_conn_error();
   echo db2_conn_errormsg();	
   echo db2_conn_errormsg();	
   echo db2_conn_errormsg();	
   echo db2_conn_errormsg();	
}

?>
--EXPECT--
db2_close succeeded
