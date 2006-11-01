<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$server = db2_server_info( $conn );

if ($server->DBMS_NAME == 'AS') {
	die('skip');
}

?> 
