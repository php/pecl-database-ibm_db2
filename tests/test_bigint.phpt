--TEST--
IBM-DB2: bigint datatype
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
    $drop_table_sql = 'drop table table0';
    $stmt = @db2_exec($conn, $drop_table_sql);

    //Create table table0 with 2 columns of type bigint
    $create_table_sql = 'create table table0( id1 bigint , id2 bigint)';
    $stmt = @db2_exec($conn, $create_table_sql);
  
    #Insert into table table0 big values	  
    $sql = 'insert into table0 values(?,?)';

    $param1 = 922337203685477580;
    $param2 = 922337203685477581;
    $param3 = 922337203685477589;

    //Prepare statement
    $prepared_stmt = db2_prepare($conn, $sql);
    db2_bind_param($prepared_stmt, 1, 'param1'); //Bind Parameter 1
    db2_bind_param($prepared_stmt, 2, 'param2'); //Bind parameter 2

    //Execute statement
    db2_execute($prepared_stmt)  ;
    //Retreive the inserted values
    $result = @db2_exec($conn,'select * from table0');
    if ($result) {
        while($row = db2_fetch_array($result)) {
            var_dump($row);
            print "\n";
        }    
    }

    $drop_proc_sql = 'drop procedure update_bigint_col';
    $stmt = @db2_exec(conn,drop_proc_sql);

    //Create procedure with 2 IN parameters of type bigint
    $create_proc_sql = "CREATE PROCEDURE update_bigint_col (IN param1 bigint, IN param2 bigint)
                        BEGIN
                            UPDATE table0 SET (id1) = (param1) WHERE id2 = param2; 
                        END";
    $stmt = @db2_exec($conn, $create_proc_sql);

    $call_sql = 'call update_bigint_col(?, ?)';

    //Prepare statement
    $prepared_stmt = db2_prepare($conn, $call_sql);
    db2_bind_param($prepared_stmt, 1, 'param3'); //Bind Parameter 1
    print db2_stmt_errormsg();
    db2_bind_param($prepared_stmt, 2, 'param2'); //Bind Parameter 2

    //Execute statement
    db2_execute($prepared_stmt);

    //Retreive the values updated through the Stored Proc
    $result = @db2_exec($conn, 'select * from table0');
    if ($result) {
        while($row = db2_fetch_array($result)) {
            var_dump($row);
          print "\n";
        }
    }
}  
?>

--EXPECT--
array(2) {
  [0]=>
  string(18) "922337203685477580"
  [1]=>
  string(18) "922337203685477581"
}

array(2) {
  [0]=>
  string(18) "922337203685477589"
  [1]=>
  string(18) "922337203685477581"
}
