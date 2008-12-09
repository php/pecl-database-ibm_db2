<?php
if (!extension_loaded('ibm_db2')) {
	die('skip');
}

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$server = db2_server_info( $conn );
$client = db2_client_info( $conn );

$minversion = "09.05.0003";

if ( ($server->DBMS_VER < $minversion) || ($client->DRIVER_VER < $minversion ) ) {
	die('skip');
}

?>
