--TEST--
IBM-DB2: db2_stmt_errormsg()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$username,$password);
db2_autocommit( $conn, DB2_AUTOCOMMIT_OFF );

if ($conn) {
    $result = @db2_exec($conn,"insert int0 t_string values(123,1.222333,'one to one')");
    if ($result) {
        $cols = db2_num_fields($result);
        echo 'col: ' . $cols . ',';
        $rows = db2_num_rows($result);
        echo 'affected row: ' . $rows ;
    }    
    else {
        echo db2_stmt_errormsg();    
    }
    $result = @db2_exec($conn,"delete from t_string where a=123");
    if ($result) {
        $cols = db2_num_fields($result);
        echo 'col: ' . $cols . ',';
        $rows = db2_num_rows($result);
        echo 'affected row: ' . $rows ;
    }    
    else {
        echo db2_stmt_errormsg();    
    }

    db2_rollback($conn);

}
else {
    echo 'no connection';    
}

?>
--EXPECTF--
Token INT0 was not valid. Valid tokens: INTO. SQLCODE=-104col: 0,affected row: 0
