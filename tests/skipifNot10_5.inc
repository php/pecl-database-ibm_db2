<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$server = db2_server_info( $conn );
if ($server->DBMS_NAME != 'AS') {
	die('skip');
}

$client = db2_client_info( $conn );
$minversion = "10.05.0000";
if ( ($client->DRIVER_VER < $minversion ) ) {
	die('skip');
}

?>
