--TEST--
IBM-DB2: Test case for inserting and retrieval of BIGINT type column.  
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if($conn)
{
    $drop = 'DROP TABLE test';
    $result = @db2_exec($conn, $drop);

    $create = 'CREATE TABLE test (id BIGINT)';
    $result = db2_exec($conn, $create);

    $insert = "INSERT INTO test values (-9223372036854775808)";
    db2_exec( $conn, $insert );

    $insert = "INSERT INTO test values (0)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (9223372036854775807)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (-5)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (5)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (-55)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (55)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (-10000000000000000)";
    db2_exec( $conn, $insert );

    $insert = "INSERT INTO test values (10000000000000000)";
    db2_exec( $conn, $insert );

    $insert = "INSERT INTO test values (999999999999999999)";
    db2_exec( $conn, $insert );

    $insert = "INSERT INTO test values (-999999999999999999)";
    db2_exec( $conn, $insert );

    $insert = "INSERT INTO test values (NULL)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (55.9)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (9223372036854775808)";
    db2_exec($conn, $insert);

    $insert = "INSERT INTO test values (-9223372036854775809)";
    db2_exec($conn, $insert);

    $stmt = db2_exec($conn, "select * from test");
    while($onerow = db2_fetch_array($stmt)) {
        var_dump($onerow[0]);
    }

    $stmt = db2_exec($conn, "select * from test");
    while(db2_fetch_row($stmt)) {
        $number = db2_result($stmt, 0);
        var_dump($number);
    }
}
else {
    echo "Connection failed.\n";
}
?>
--EXPECTF--
Warning: db2_exec(): Statement Execute Failed in %s

Warning: db2_exec(): Statement Execute Failed in %s
string(20) "-9223372036854775808"
string(1) "0"
string(19) "9223372036854775807"
string(2) "-5"
string(1) "5"
string(3) "-55"
string(2) "55"
string(18) "-10000000000000000"
string(17) "10000000000000000"
string(18) "999999999999999999"
string(19) "-999999999999999999"
NULL
string(2) "55"
string(20) "-9223372036854775808"
string(1) "0"
string(19) "9223372036854775807"
string(2) "-5"
string(1) "5"
string(3) "-55"
string(2) "55"
string(18) "-10000000000000000"
string(17) "10000000000000000"
string(18) "999999999999999999"
string(19) "-999999999999999999"
NULL
string(2) "55"
