--TEST--
IBM-DB2: db2_server_info()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$server = db2_server_info( $conn );

if ($server) {
    echo "DBMS_NAME: "; 		var_dump( $server->DBMS_NAME );	 
    echo "DBMS_VER: "; 			var_dump( $server->DBMS_VER );	 
    // echo "DB_CODEPAGE: "; 		// var_dump( $server->DB_CODEPAGE );	 
    echo "DB_NAME: "; 			var_dump( $server->DB_NAME );	 
    // echo "INST_NAME: "; 		// var_dump( $server->INST_NAME );	 
    // echo "SPECIAL_CHARS: "; 		// var_dump( $server->SPECIAL_CHARS );	 
    echo "KEYWORDS: "; 			var_dump( sizeof($server->KEYWORDS) );	 
    echo "DFT_ISOLATION: "; 		var_dump( $server->DFT_ISOLATION );	 
    // echo "ISOLATION_OPTION: "; 		
    // $il = '';
    // foreach( $server->ISOLATION_OPTION as $opt )
    // {
    //    $il .= $opt." ";
    // }
    // var_dump( $il );	 
    echo "SQL_CONFORMANCE: "; 		var_dump( $server->SQL_CONFORMANCE );	 
    echo "PROCEDURES: "; 		var_dump( $server->PROCEDURES );	 
    echo "IDENTIFIER_QUOTE_CHAR: ";	var_dump( $server->IDENTIFIER_QUOTE_CHAR );	 
    echo "LIKE_ESCAPE_CLAUSE: "; 	var_dump( $server->LIKE_ESCAPE_CLAUSE );	 
    echo "MAX_COL_NAME_LEN: "; 		var_dump( $server->MAX_COL_NAME_LEN );	 
    echo "MAX_ROW_SIZE: "; 		var_dump( $server->MAX_ROW_SIZE );	 
    // echo "MAX_IDENTIFIER_LEN: "; 	// var_dump( $server->MAX_IDENTIFIER_LEN );	 
    // echo "MAX_INDEX_SIZE: "; 	// var_dump( $server->MAX_INDEX_SIZE );	 
    // echo "MAX_PROC_NAME_LEN: "; 	// var_dump( $server->MAX_PROC_NAME_LEN );	 
    echo "MAX_SCHEMA_NAME_LEN: "; 	var_dump( $server->MAX_SCHEMA_NAME_LEN );	 
    echo "MAX_STATEMENT_LEN: "; 	var_dump( $server->MAX_STATEMENT_LEN );	 
    echo "MAX_TABLE_NAME_LEN: "; 	var_dump( $server->MAX_TABLE_NAME_LEN );	 
    echo "NON_NULLABLE_COLUMNS: "; 	var_dump( $server->NON_NULLABLE_COLUMNS );	 

    db2_close($conn);
}
else {
    echo "Error.";
}

?>
--EXPECTF--
DBMS_NAME: string(%d) %s
DBMS_VER: string(%d) %s
DB_NAME: string(%d) %s
KEYWORDS: int(%d)
DFT_ISOLATION: string(%d) %s
SQL_CONFORMANCE: string(%d) %s
PROCEDURES: bool(%s)
IDENTIFIER_QUOTE_CHAR: string(%d) %s
LIKE_ESCAPE_CLAUSE: bool(%s)
MAX_COL_NAME_LEN: int(%d)
MAX_ROW_SIZE: int(%d)
MAX_SCHEMA_NAME_LEN: int(%d)
MAX_STATEMENT_LEN: int(%d)
MAX_TABLE_NAME_LEN: int(%d)
NON_NULLABLE_COLUMNS: bool(%s)

