--TEST--
IBM-DB2: 1.9.7 - LUW and IBM i CRTLIB isolation *NONE
--SKIPIF--
<?php require_once('skipifNot10_5.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
/*
========
db2 connect 10.5+
db2inst2 -> linux install db2 connect instance
lp0364d -> IBM i
========

>su db2inst2
>db2
db2 => catalog tcpip node LP0364D remote lp0364d.rch.stglabs.ibm.com server 446
db2 => catalog db LP0364D at node LP0364D authentication dcs
db2 => catalog dcs db LP0364D as LP0364D
db2 => terminate
Note: "LP0364D" - can be found using WRKRDBDIRE (local RDB name / database name)

ERROR: if you see SQL0805N  Package "NULLID.SYSSH000" was not found

>su db2inst2
$ db2
db2 => connect to lp0364d user db2 using xxxxx 
db2 => bind "/opt/ibm/db2/V10.5/bnd/@db2cli.lst" blocking all grant public
db2 => terminate
Note: On IBM i CRTLIB NULLID (if not exist, before bind)
*/


$conn = db2_connect($database, $user, $password, array('i5_commit'=>DB2_I5_TXN_NO_COMMIT));
if (!$conn) die("connect\n".db2_stmt_errormsg()."\n");

// crtlib no journal, only update via DB2_I5_TXN_NO_COMMIT
$crtlib = "CRTLIB LIB(DB2NONE)";
$crtlen = strlen($crtlib);
$qcmdexe = "CALL QSYS2.QCMDEXC('{$crtlib}',{$crtlen})";
$res = @db2_exec($conn, $qcmdexe);
echo "qcmdexe\n".db2_stmt_errormsg()."\n";

//Drop the animal table, in case it exists
$drop = "DROP TABLE DB2NONE.ANIMALS";
$res = @db2_exec($conn, $drop);
echo "drop\n".db2_stmt_errormsg()."\n";

// Create the animal table
$create = "CREATE TABLE DB2NONE.ANIMALS (ID INTEGER, BREED VARCHAR(32), NAME CHAR(16), WEIGHT DECIMAL(7,2))";
$res = db2_exec($conn, $create);
if (!$res) die("create\n".db2_stmt_errormsg()."\n");

//Populate the animal table
$pets = array(
  array(0, 'cat',        'Pook',         3.2),
  array(1, 'dog',        'Peaches',      12.3),
  array(2, 'horse',      'Smarty',       350.0),
  array(3, 'gold fish',  'Bubbles',      0.1),
  array(4, 'budgerigar', 'Gizmo',        0.2),
  array(5, 'goat',       'Rickety Ride', 9.7),
  array(6, 'llama',      'Sweater',      150)
);
$insert = "INSERT INTO DB2NONE.ANIMALS (ID, BREED, NAME, WEIGHT) VALUES (?,?,?,?)";
$sth = db2_prepare($conn, $insert);
if (!$sth) die("prepare\n".db2_stmt_errormsg()."\n");
foreach($pets as $pet){
   $res = db2_execute($sth, $pet);
   if (!$res) die("execute insert\n".db2_stmt_errormsg()."\n");
}

echo "success\n";

?>
--EXPECTF--
%s
success

