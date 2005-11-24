<?php

$database = 'SAMPLE';
$user = 'db2inst1';
$password = 'ibmdb2';
$hostname = 'localhost';
$port = 50000;

$db = $database;
$username = $user;

$prepconn = db2_connect($database, $user, $password);
require('prepare.inc');
db2_close($prepconn);
?>
