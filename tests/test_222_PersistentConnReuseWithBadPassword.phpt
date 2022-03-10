--TEST--
IBM-DB2: db2_pconnect() - test persistent connection won't be reused with bad password
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

/*
 * Use blatantly incorrect password, to make sure password is part of the
 * internal persistent connection hash.
 */
$conn1 = db2_pconnect($database, $user, $password);
if ($conn1) {
    $conn2 = db2_pconnect($database, $user, "wrongbad");
    if ($conn2) {
        echo "A bad password was accepted.\n";
        db2_close($conn2);
    } else {
        echo "OK\n";
    }
    db2_close($conn1);
}
else {
    echo "Connection failed.\n";
}

?>
--EXPECT--
OK
