--TEST--
IBM-DB2: db2_fetch_into several rows
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

if ($conn) {
    require_once('prepare.inc');
    $stmt = db2_exec( $conn, "select * from animals order by breed" );

    $i = 0;

    while( $cols = db2_fetch_into( $stmt ) ) {
        foreach ($cols as $col) {
            print "$col ";
        }
        print "\n";
        $i++;
    }

    echo "\nNumber of rows: ".$i;
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
4 budgerigar Gizmo            0.20 
0 cat Pook             3.20 
1 dog Peaches          12.30 
5 goat Rickety Ride     9.70 
3 gold fish Bubbles          0.10 
2 horse Smarty           350.00 
6 llama Sweater          150.00 

Number of rows: 7
