--TEST--
IBM-DB2: pConnect Rollback Handler Test.
--SKIPIF--
<?php require_once('rollbackhandler.inc'); ?>
--FILE--
<?php
  require_once('connection.inc');

  $conn = db2_connect($database, $user, $password);

  db2_autocommit($conn,DB2_AUTOCOMMIT_ON);

  function prepare() {
    global $conn;

    $stmtCreate = db2_exec($conn,"create table rollbacktest(id integer primary key not null, charval varchar(20))");
    
    if($stmtCreate) {
      return 1;
    } else {
      echo "creation of table failed\n";
      $err = db2_stmt_errormsg();
      print "done $err";
      return 0;
    }
  }

  function cleanup() {
    global $conn;

    $stmtdrop = db2_exec($conn,"drop table rollbacktest");

    if($stmtdrop) {
      return 1;
    } else {
      echo "drop table failed\n";
      return 0;
    }
  }

  function callURL() {
    global $url,$database,$user,$password,$hostname,$port;
    $myurl = "$url?database=$database&user=$user&password=$password&host=$hostname&port=$port";
    $ch = curl_init($myurl);
    if( $ch ) {
      curl_exec($ch);
    } else {
      echo "URL specified is not valid";
    }
  }

  function checkTest() {
    global $conn;
    $stmt = db2_exec($conn,"insert into rollbacktest (id,charval) values (1,'rolled back')");
    if ( $stmt) {
      echo "Test successful\n";
    } else {
       echo "Test failed\n";
       $err = db2_stmt_errormsg();
       print "$err";
    }
  }

  function main() {
    $retVal = prepare();
    if( $retVal ){
      callURL();
      checkTest();
      cleanup();
    } else {
      echo "DB preapre failed \n";
    }
  }

  if ($conn) {
     echo "Connection succeeded.\n" ;
     main();
  } else {
     echo "Connection failed. \n";
     print db2_conn_errormsg($conn);
  }
?>
--EXPECT--
Connection succeeded.
Connection succeeded.
row insert went through successfully
Test successful
