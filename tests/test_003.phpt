--TEST--
IBM-DB2: Count number of affected rows
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $sql = 'UPDATE animals SET id = 9';
    $res = db2_exec($conn, $sql);
    print "Number of affected rows: " . db2_num_rows($res);
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Number of affected rows: 7
