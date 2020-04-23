--TEST--
IBM-DB2: 1.9.7 - IBM i Hebrew (zend support issue)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$input = 'IBM i data string "123Eאבג"';
$drop = 'DROP TABLE HEBREW';
$res = @db2_exec($conn, $drop);
$create = 'CREATE TABLE HEBREW (TEXTME VARCHAR(1024) CCSID  62211)'; // not use 424
$res = db2_exec($conn, $create);
$insert = 'INSERT INTO HEBREW (TEXTME) VALUES (?)';
$sth = db2_prepare($conn, $insert);
$res = db2_execute($sth, array($input));
$look = 'select TEXTME from HEBREW';
$res = db2_exec($conn, $look);
$row = db2_fetch_array($res);
echo $row[0];
echo "\n";
echo $input;
echo "\n";

if (bin2hex($row[0]) == bin2hex($input)) {
	echo "success\n";
} else {
	echo "Failure]n";
}

/*
http://www-01.ibm.com/support/docview.wss?uid=nas8N1010892
To resolve this, you should use the CCSID 62211 in place of 424. CCSID 62211 is ST 5. 
The conversion from 62211 to 1200 or 1208 (ST 5 to ST 10) results in no reordering. 
*/

?>
--EXPECTF--
IBM i data string "123Eאבג"
IBM i data string "123Eאבג"
success

