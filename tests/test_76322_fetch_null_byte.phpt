--TEST--
IBM-DB2: PECL bug 76322 -- strings containing null-byte get truncated
--SKIPIF--
<?php
require_once('skipif3.inc');
?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);
$stmt = db2_prepare($conn, "SELECT X'410042' V FROM sysibm.sysdummy1");
db2_execute($stmt);
$row = db2_fetch_both($stmt);

echo bin2hex($row[0]), PHP_EOL;
echo bin2hex($row['V']), PHP_EOL;

?>
--EXPECT--
410042
410042
