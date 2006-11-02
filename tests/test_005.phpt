--TEST--
IBM-DB2: db2_connect() - bad user / bad pwd
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$baduser = "non_user";
$badpass = "invalid_password";
$dsn = "DATABASE=$db;UID=$baduser;PWD=$badpass;";
$conn = db2_connect($dsn, "", "");
if ( $conn )
{
   print "odd, db2_connect succeeded with an invalid user / password\n";
   db2_close($conn);
}
else 
   echo "Ooops";

?>
--EXPECT--
Ooops
