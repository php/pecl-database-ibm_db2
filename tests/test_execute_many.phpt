--TEST--
IBM-DB2: db2_execute_many - insert
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
    // Drop the tabmany table, in case it exists
    $drop = "DROP TABLE TABMANY";
    $res = @db2_exec($conn, $drop);
    
    // Create table tabmany
    $create = "CREATE TABLE TABMANY(id SMALLINT NOT NULL, name VARCHAR(32))";
    $stmt = db2_exec($conn, $create);
    
    // Populate the tabmany table with execute_many
    $insert = "INSERT INTO TABMANY (id, name) VALUES(?, ?)";
    $params = array(
                array(10, 'Sanders'), 
                array(20, 'Pernal'), 
                array(30, 'Marenghi'), 
                array(40, 'OBrien'));
    $stmt_insert = db2_prepare($conn, $insert);
    $effected_rows = db2_execute_many($stmt_insert, $params);
    if ($effected_rows) {
        echo "Number of affected rows:" . $effected_rows . "\n";
    }
    else {
        echo db2_stmt_errormsg($stmt_insert);
    }
    
    // chaeck the inserted columns
    $select = "SELECT * FROM TABMANY";
    $stmt = db2_exec($conn, $select);
    while (db2_fetch_row($stmt) == TRUE) {
        $id = db2_result($stmt, 0);
        var_dump($id);
        $name = db2_result($stmt, 1);
        var_dump($name);
    }
    
    // populate the tabmany table
    $params = array(
                array(50, 'Hanes'), 
                array(55, ), 
                array('60', 'invalid row'), 
                array( 65, 'Quigley'),
                array( 70, NULL));
                
    $effected_rows = db2_execute_many($stmt_insert, $params);
    
    if ( !$effected_rows ) {
        echo db2_stmt_errormsg($stmt_insert);
        echo "Number of effected rows are: " . db2_num_rows( $stmt_insert );
    }
    
    db2_close($conn);
} else {
    echo "Connection failed.\n";
}
?>
--EXPECT--
Number of affected rows:4
int(10)
string(7) "Sanders"
int(20)
string(6) "Pernal"
int(30)
string(8) "Marenghi"
int(40)
string(6) "OBrien"
ERROR 1: Value parameters array 2 has less number of parameteres
ERROR 2: Value parameters array 3 is not homogeneous with privious parameters array
Number of effected rows are: 3
