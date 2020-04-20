--TEST--
IBM-DB2: db2_tables() - 2
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);

$schema = 'CREATE SCHEMA t';
$result = @db2_exec($conn, $schema);

$create = 'CREATE TABLE t.t1( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$create = 'CREATE TABLE t.t2( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$create = 'CREATE TABLE t.t3( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$create = 'CREATE TABLE t.t4( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

if ($conn) 
{
   $result = db2_tables($conn,NULL,strtoupper('t'));	
   $i = 0;
   while ($row = db2_fetch_both($result)) 
   {
      if (preg_match("/T1|T2|T3|T4/i",$row['TABLE_NAME'])) {			
      $str = $row['TABLE_SCHEM'] . $row['TABLE_NAME'] . $row['TABLE_TYPE'];
      if ($i < 4) print $str . "\n";
      $i++;
      }
   }

   @db2_exec($conn, 'DROP TABLE t.t1');
   @db2_exec($conn, 'DROP TABLE t.t2');
   @db2_exec($conn, 'DROP TABLE t.t3');
   @db2_exec($conn, 'DROP TABLE t.t4');

   print "done!";
}
else 
{
   echo 'no connection: ' . db2_conn_errormsg();	
}

?>
--EXPECT--
TT1TABLE
TT2TABLE
TT3TABLE
TT4TABLE
done! 
