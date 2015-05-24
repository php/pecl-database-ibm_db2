--TEST--
IBM-DB2: CLOB, special character test.
--SKIPIF--
<?php require_once('skipif3.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
	echo "Connection succeeded.\n";

    $sql = "DROP TABLE CLOB_TEST";
    $rc = @db2_exec($conn, $sql);

    $sql = "CREATE TABLE CLOB_TEST (col_clob CLOB)";
    $rc = @db2_exec($conn, $sql);

    $sql = "INSERT INTO CLOB_TEST VALUES('I have 100€.')";
    $rc = @db2_exec($conn, $sql);

    $sql= "Select * from CLOB_TEST";

    $stmt= db2_prepare($conn, $sql);
    $rc= db2_execute($stmt);
    while($result= db2_fetch_array($stmt)) {
    	var_dump( bin2hex($result[0]) );
    	var_dump( $result );
    }
    
    db2_close($conn);
}
else {
    echo "Connection failed.";
}

?>
--EXPECT--
Connection succeeded.
string(24) "49206861766520313030802e"
array(1) {
  [0]=>
  string(12) "I have 100€."
}
