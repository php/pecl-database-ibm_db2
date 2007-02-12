<?php

$database = 'sample';
$user = 'db2inst1';
$password = 'password';
$hostname = 'localhost';
$port = 60000;

$db = $database;
$username = $user;

$prepconn = db2_connect($database, $user, $password);
db2_close($prepconn);
?>
