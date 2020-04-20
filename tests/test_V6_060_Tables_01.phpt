--TEST--
IBM-DB2: db2_tables() - 1
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
   while ($row=db2_fetch_both($result))
   {
      if (preg_match("/T1|T2|T3|T4/i",$row[2])) {			
      if ($i < 4) print $row[1] . "/" . $row[2] . "\n";
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
T/T1
T/T2
T/T3
T/T4
done! 
