<?php

$database = 'sample';
$user = 'db2inst1';
$password = 'password';
$hostname = 'localhost';
$port = 60000;

$db = $database;
$username = $user;

// test connection ok
$prepconn = db2_connect($database, $user, $password);
if (!$prepconn) die('skip');

// control skipif.inc
$server = db2_server_info($prepconn);
if ($server->DBMS_NAME == "QSQ" || $server->DBMS_NAME == "AS") {
  if ($server->DBMS_VER > "06000") $rel = "V6";
  else $rel = "/V5/";
  // PASE test_xxx_V5V6.phpt
  if (preg_match("/V6|V5/", $_SERVER["PHP_SELF"])) {
    if (!preg_match($rel, $_SERVER["PHP_SELF"])) {
      die('skip');
    }
  }
  // check the skipif.PASE
  else {
    $result=file_get_contents("skipif.PASE");
    for ($line = strtok($result, "\n"); $line; $line = strtok("\n")) {
      $pos=strpos($line,"PASE");
      if ($pos && strpos($line, "|$rel|")){
        $check = substr($line, 1, $pos-6);
        if (strpos($_SERVER["PHP_SELF"], $check)) {
          die('skip');
        }
      }
    }
  }
}
else {
  if (preg_match("/V5|V6/", $_SERVER["PHP_SELF"])) die('skip');
}

db2_close($prepconn);
?>
