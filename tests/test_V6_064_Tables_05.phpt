--TEST--
IBM-DB2: db2_tables() - 5
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

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

$result = db2_tables($conn, NULL, "T");

for ($i=0; $i<db2_num_fields($result); $i++) 
{
   print db2_field_name($result, $i) . ", ";	
}
print "\n\n";

$i = 0;
while ($row=db2_fetch_array($result))
{
    if (preg_match("/T1|T2|T3|T4/i",$row[2])) {
    db2_num_fields($result);
    if ($i < 4) print ", " . $row[1] . ", " . $row[2] . ", " . $row[3] . ", , \n";
    $i++;
    }
}

db2_free_result($result);

@db2_exec($conn, 'DROP TABLE t.t1'); 
@db2_exec($conn, 'DROP TABLE t.t2'); 
@db2_exec($conn, 'DROP TABLE t.t3'); 
@db2_exec($conn, 'DROP TABLE t.t4'); 

?>
--EXPECT--
TABLE_CAT, TABLE_SCHEM, TABLE_NAME, TABLE_TYPE, REMARKS, 

, T, T1, TABLE, , 
, T, T2, TABLE, , 
, T, T3, TABLE, , 
, T, T4, TABLE, ,
