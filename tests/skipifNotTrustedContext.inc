<?php
if (!extension_loaded('ibm_db2')) {
	die('skip');
}

require_once('trusted_connection.inc');

if (strlen($authID) == 0 || strlen($auth_pass) == 0 || strlen($tcuser) == 0 || strlen($tcuser_pass) == 0) {
	die('skip');
}

$prepconn = db2_connect($database, $authID, $auth_pass);
if (!$prepconn) {
	die('skip');
}
db2_close($prepconn);

$prepconn = db2_connect($database, $tcuser, $tcuser_pass);
if (!$prepconn) {
	die('skip');
}
db2_close($prepconn);
?>
