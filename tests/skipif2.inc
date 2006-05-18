<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$server = db2_server_info( $conn );

if (substr($server->DBMS_VER, 1, 2) == '08') {
	die('skip');
}

?> 