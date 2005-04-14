--TEST--
IBM-DB2: db2_tables() - 5
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$create = 'CREATE TABLE t.t1( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$create = 'CREATE TABLE t.t2( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$create = 'CREATE TABLE t.t3( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$create = 'CREATE TABLE t.t4( c1 integer, c2 varchar(40))';
$result = @db2_exec($conn, $create);

$result = db2_tables($conn, NULL, "T");

for ($i=0; $i<db2_num_fields($result); $i++) 
{
   print db2_field_name($result, $i) . ", ";	
}
print "\n\n";

while ($row=db2_fetch_into($result))
{
   for ($i=0; $i<db2_num_fields($result); $i++) 
   {
      print $row[$i] . ", ";	
   }
   print "\n";
}

db2_free_result($result);

?>
--EXPECT--
TABLE_CAT, TABLE_SCHEM, TABLE_NAME, TABLE_TYPE, REMARKS, 

, T, T1, TABLE, , 
, T, T2, TABLE, , 
, T, T3, TABLE, , 
, T, T4, TABLE, ,