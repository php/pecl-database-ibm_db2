--TEST--
IBM-DB2: 1.9.7 - LUW and IBM i date and time formats
--SKIPIF--
<?php require_once('skipifNot10_5.inc'); ?>
--FILE--
<?php
require_once('connection.inc');

$all = array();

$conn1 = db2_connect($database, $user, $password, 
array(
'i5_naming'=>DB2_I5_NAMING_ON, 
'i5_libl'=>"$user", 
'i5_curlib'=>"$user"
)
);
if (!$conn1) die("connect\n".db2_stmt_errormsg()."\n");

//Drop the dateme table, in case it exists
$drop = "DROP TABLE DATEME";
$res = @db2_exec($conn1, $drop);
echo "drop\n".db2_stmt_errormsg()."\n";

// Create the dateme table
$create = "CREATE TABLE DATEME (ID DECIMAL(7, 2), col1 DATE DEFAULT CURRENT DATE, col2 TIME DEFAULT CURRENT TIME)";
$res = db2_exec($conn1, $create);
if (!$res) die("create\n".db2_stmt_errormsg()."\n");

//Populate the dateme table
$insert = "INSERT INTO DATEME (ID) values(?)";
$sth = db2_prepare($conn1, $insert);
if (!$sth) die("prepare\n".db2_stmt_errormsg()."\n");
for($i=0;$i<9;$i++){
   $res = db2_execute($sth, array(10000 + $i+(.11*$i)));
   if (!$res) die("execute insert\n".db2_stmt_errormsg()."\n");
}
$stmt = db2_exec( $conn1, "SELECT ID,col1,col2 FROM DATEME" );
if (!$stmt) die("stmt\n".db2_stmt_errormsg()."\n");
while ($row = db2_fetch_array($stmt)) {
// var_dump($row);
$all[0][]=implode(" ",$row);
}

$conn2 = db2_connect($database, $user, $password, 
array(
'i5_naming'=>DB2_I5_NAMING_ON, 
'i5_libl'=>"$user", 
'i5_curlib'=>"$user",
'i5_date_fmt'=>DB2_I5_FMT_DMY,
'i5_date_sep'=>DB2_I5_SEP_SLASH,
'i5_time_fmt'=>DB2_I5_FMT_HMS,
'i5_time_sep'=>DB2_I5_SEP_BLANK
)
);
if (!$conn2) die("connect\n".db2_stmt_errormsg()."\n");
$stmt = db2_exec( $conn2, "SELECT ID,col1,col2 FROM DATEME" );
if (!$stmt) die("stmt\n".db2_stmt_errormsg()."\n");
while ($row = db2_fetch_array($stmt)) {
// var_dump($row);
$all[1][]=implode(" ",$row);
}

$conn3 = db2_connect($database, $user, $password, 
array(
'i5_naming'=>DB2_I5_NAMING_ON, 
'i5_libl'=>"$user", 
'i5_curlib'=>"$user",
'i5_date_fmt'=>DB2_I5_FMT_EUR,
'i5_time_fmt'=>DB2_I5_FMT_USA
)
);
if (!$conn3) die("connect\n".db2_stmt_errormsg()."\n");
$stmt = db2_exec( $conn3, "SELECT ID,col1,col2 FROM DATEME" );
if (!$stmt) die("stmt\n".db2_stmt_errormsg()."\n");
while ($row = db2_fetch_array($stmt)) {
// var_dump($row);
$all[2][]=implode(" ",$row);
}


for ($i=0;$i<9;$i++) {
  if ($all[0][$i] == $all[1][$i]) {
    die("fail {$all[0][$i]} {$all[1][$i]}\n");
  }
  if ($all[0][$i] == $all[2][$i]) {
    die("fail {$all[0][$i]} {$all[2][$i]}\n");
  }
  if ($all[1][$i] == $all[2][$i]) {
    die("fail {$all[0][$i]} {$all[1][$i]}\n");
  }
  echo("good {$all[0][$i]} {$all[1][$i]} {$all[2][$i]}\n");
}
echo "success\n";
?>
--EXPECTF--
%s
success

