--TEST--
IBM-DB2: db2_connect() - wrong user/pwd
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect( "sample", "not_a_user", "inv_pass");
if (!$conn) {
    print "connect failed, test succeeded\n";
} else {
    print "connect succeeded? Test failed\n";
}

?>
--EXPECT--
connect failed, test succeeded
