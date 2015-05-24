--TEST--
IBM-DB2: 1.9.7 - IBM i security restrict blank db,uid,pwd (unless customer allow flag)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');

putenv("IBM_DB_I5_TEST=1");
putenv("IBM_DB_i5_blank_userid=0");
$conn = db2_connect('', '', '');
if ($conn) 
{
    echo "success\n";
}
else {
    echo "failed\n";
}
putenv("IBM_DB_i5_blank_userid=1");
$conn = db2_connect('', '', '');
if ($conn) 
{
    echo "success\n";
}
else {
    echo "failed\n";
}

?>
--EXPECT--
failed
success

