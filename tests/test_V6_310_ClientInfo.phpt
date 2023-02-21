--TEST--
IBM-DB2: db2_client_info()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$client = db2_client_info( $conn );

if ($client instanceof \stdClass) {
    echo "DRIVER_NAME: "; 		var_dump( $client->DRIVER_NAME );	 
    echo "DRIVER_VER: "; 		var_dump( $client->DRIVER_VER );	 
    echo "DATA_SOURCE_NAME: "; 		var_dump( $client->DATA_SOURCE_NAME );	 
    echo "DRIVER_ODBC_VER: "; 		var_dump( $client->DRIVER_ODBC_VER );	 
    // echo "ODBC_VER: "; 		// var_dump( $client->ODBC_VER );	 
    echo "ODBC_SQL_CONFORMANCE: "; 	var_dump( $client->ODBC_SQL_CONFORMANCE );	 
    // echo "APPL_CODEPAGE: "; 		// var_dump( $client->APPL_CODEPAGE );	 
    // echo "CONN_CODEPAGE: "; 		// var_dump( $client->CONN_CODEPAGE );	 

    db2_close($conn);
}
else {
    echo "Error.";
}

?>
--EXPECTF--
DRIVER_NAME: string(%d) %s
DRIVER_VER: string(%d) %s
DATA_SOURCE_NAME: string(%d) %s
DRIVER_ODBC_VER: string(%d) %s
ODBC_SQL_CONFORMANCE: string(%d) %s
