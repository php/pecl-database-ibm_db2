--TEST--
IBM-DB2: decfloat datatype
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
    //Drop the STOCKPRICE table, in case it exists
    $drop = 'DROP TABLE STOCKPRICE';
    $res = @db2_exec($conn, $drop);

    //Create the STOCKPRICE table
    $create = 'CREATE TABLE STOCKPRICE (id SMALLINT NOT NULL, company VARCHAR(30), Stockshare DECIMAL(7, 2), stockprice DECFLOAT(16))';
    $res = db2_exec($conn, $create);

	// Insert Directly
    $insert = "INSERT INTO STOCKPRICE (id, company, Stockshare, stockprice) VALUES (10, 'Megadeth', 100.002, 990.356736488388374888532323)";
    $res = db2_exec($conn, $insert);
    
    //Populate the STOCKPRICE table
    $stockprice = array(
                   array(20, "Zaral", 102.205, "100.234"),
                   array(30, "Megabyte", 98.65, "1002.112"),
                   array(40, "Visarsoft", 123.34, "1652.345"),
                   array(50, "Mailersoft", 134.22, "1643.126"),
                   array(60, "Kaerci", 100.97, "9876.765")
                    );
    $insert = 'INSERT INTO STOCKPRICE (id, company, Stockshare, stockprice) VALUES (?, ?, ?, ?)';
    $sth = db2_prepare($conn, $insert);
    if($sth) {
       foreach($stockprice as $row){
          $res = db2_execute($sth, $row);
       }
    }

	$id = 70;
    $company = 'Nirvana';
    $stockshare = 100.1234;
    $stockprice = "100.567";

    db2_bind_param($sth, 1, 'id');
    db2_bind_param($sth, 2, 'company');
    db2_bind_param($sth, 3, 'stockshare');
    db2_bind_param($sth, 4, 'stockprice');
    db2_execute($sth);    
    
    // Select the result from the table and display with expected results
    $query = 'SELECT * FROM STOCKPRICE ORDER BY id';
    $stmt = db2_prepare($conn, $query);
    if (db2_execute($stmt)) {
        while ($row = db2_fetch_both($stmt)) {
            printf("%s : %s : %s : %s\n", $row[0], $row[1], $row[2], $row[3]);
        }
    }
}

?>
--EXPECT--
10 : Megadeth : 100.00 : 990.3567364883884
20 : Zaral : 102.20 : 100.234
30 : Megabyte : 98.65 : 1002.112
40 : Visarsoft : 123.34 : 1652.345
50 : Mailersoft : 134.22 : 1643.126
60 : Kaerci : 100.97 : 9876.765
70 : Nirvana : 100.12 : 100.567