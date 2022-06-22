--TEST--
IBM-DB2: db2_columns() - 1 - table
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

if ($conn) 
{
   $result = @db2_columns($conn,NULL,NULL,"EMPLOYEE");
   while ($row = db2_fetch_array($result))
   {
      $str = $row[1] ."/". $row[3];	
      print $str . "\n";
   }
   print "done!";
}
else
{
   echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECTF--
%s/EMPNO
%s/FIRSTNME
%s/MIDINIT
%s/LASTNAME
%s/WORKDEPT
%s/PHONENO
%s/HIREDATE
%s/JOB
%s/EDLEVEL
%s/SEX
%s/BIRTHDATE
%s/SALARY
%s/BONUS
%s/COMM
done! 
