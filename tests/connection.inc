<?php

$database = 'sample';
$user = 'db2inst1';
$password = 'password';
$hostname = 'localhost';
$port = 60000;
$url = 'no url';
$switch_user = '';
$switch_password = '';

$db = $database;
$username = $user;

// IBM i (PASE)
$myos = PHP_OS;
if (strpos('x'.$myos,'AIX') > 0) {
  $myos = shell_exec("uname");
}
if (strpos($myos,'400') > 0) {
  if (strpos($_SERVER["PHP_SELF"],"V6") === false) {
    require_once('skipif.PASE');
    for ($entry = strtok($PASE_ALT_TEST,"\n"); $entry; $entry = strtok("\n")) {
      $check = str_replace(array("test_V6",".phpt",".php"),"",$entry);
      if (strpos($_SERVER["PHP_SELF"],$check) > 0) {
        die('skip');
      }
    }
  }
}
// LUW (not PASE) 
else {
  if (strpos($_SERVER["PHP_SELF"],"V6") > 0) {
    die('skip');
  }
}


date_default_timezone_set('UTC');


$skip_on_connect_failure = getenv("IBM_DB2_TEST_SKIP_CONNECT_FAILURE") !== FALSE ?
                            getenv("IBM_DB2_TEST_SKIP_CONNECT_FAILURE") : true;


// test connection ok
$prepconn = db2_connect($database, $user, $password);
if (!$prepconn) {
  if($skip_on_connect_failure) die("skip - Couldn't connect");
}
else {
  db2_close($prepconn);
}

unset($skip_on_connect_failure);

?>
