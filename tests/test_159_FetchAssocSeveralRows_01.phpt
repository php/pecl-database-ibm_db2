--TEST--
IBM-DB2: db2_fetch_assoc() - several rows
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "select name,job from staff");
$i=1;
while ($row = db2_fetch_assoc($result)) 
{
   printf ("%3d %10s %10s\n",$i, $row['NAME'], $row['JOB']);
   $i++;
}


?>
--EXPECT--
  1    Sanders      Mgr  
  2     Pernal      Sales
  3   Marenghi      Mgr  
  4    O'Brien      Sales
  5      Hanes      Mgr  
  6    Quigley      Sales
  7    Rothman      Sales
  8      James      Clerk
  9    Koonitz      Sales
 10      Plotz      Mgr  
 11       Ngan      Clerk
 12   Naughton      Clerk
 13  Yamaguchi      Clerk
 14      Fraye      Mgr  
 15   Williams      Sales
 16   Molinare      Mgr  
 17   Kermisch      Clerk
 18   Abrahams      Clerk
 19    Sneider      Clerk
 20   Scoutten      Clerk
 21         Lu      Mgr  
 22      Smith      Sales
 23  Lundquist      Clerk
 24    Daniels      Mgr  
 25    Wheeler      Clerk
 26      Jones      Mgr  
 27        Lea      Mgr  
 28     Wilson      Sales
 29      Quill      Mgr  
 30      Davis      Sales
 31     Graham      Sales
 32   Gonzales      Sales
 33      Burke      Clerk
 34    Edwards      Sales
 35     Gafney      Clerk
