--TEST--
IBM-DB2: Trusted Context with db2_pconnect
--SKIPIF--
<?php require_once('skipifNotTrustedContext.inc'); ?>
--FILE--
<?php
require_once('trusted_connection.inc');

$sql_drop_role = "DROP ROLE role_01";
$sql_create_role = "CREATE ROLE role_01";

$sql_drop_trusted_context = "DROP TRUSTED CONTEXT ctx";	

$sql_create_trusted_context = "CREATE TRUSTED CONTEXT ctx BASED UPON CONNECTION USING SYSTEM AUTHID ";
$sql_create_trusted_context .= $authID;
$sql_create_trusted_context .= " ATTRIBUTES (ADDRESS '";
$sql_create_trusted_context .= $hostname;
$sql_create_trusted_context .= "') DEFAULT ROLE role_01 ENABLE WITH USE FOR ";
$sql_create_trusted_context .= $tcuser;

$userBefore = null;
$userAfter = null;

//Creating set up for trusted context WITH AUTHENTICATION
$conn_01 = db2_connect($database, $user, $password);

if ($conn_01) {
	echo "Connection succeeded. Creating new Trusted Context (WITH Authentication).\n";
	
	//Dropping Trusted Context.	
	$res = @db2_exec($conn_01, $sql_drop_trusted_context);

	//Dropping Role.	
	$res = @db2_exec($conn_01, $sql_drop_role);
	
	//Creating Role.	
	$res = @db2_exec($conn_01, $sql_create_role);
	
	create_database($conn_01);
	print_table($conn_01, "SELECT * FROM trusted_table");
	
	//Granting permissions to role.
	$sql_grant_permission = "GRANT INSERT ON TABLE trusted_table TO ROLE role_01";
	$res = @db2_exec($conn_01, $sql_grant_permission);
	
	//Creating Trusted Context for user with authentication.
	$sql_create_trusted_context_01 = $sql_create_trusted_context . " WITH AUTHENTICATION";
	$res = db2_exec($conn_01, $sql_create_trusted_context_01);
	
	db2_close($conn_01);
}
else {
    echo "Connection failed.\n";
}
$conn_01 = null;

/*
 * Testing trusted connection.
 */

$options = array ("trustedcontext" => DB2_TRUSTED_CONTEXT_ENABLE);
$connString = "DATABASE=$database;HOSTNAME=$hostname;PORT=$port;PROTOCOL=TCPIP;UID=$authID;PWD=$auth_pass;";
$tc_conn_01 = db2_pconnect($connString, "", "", $options);
if($tc_conn_01) {
	echo "Explicit Trusted Connection succeeded.\n";
	echo "Performing normal operations with trusted connection.\n";

	if(db2_get_option($tc_conn_01, "trustedcontext") == 1) {
		$userBefore = db2_get_option($tc_conn_01, "trusted_user");

		//Switching to trusted user.
		$parameters = array("trusted_user" => $tcuser, "trusted_password" => $tcuser_pass);
		$res = db2_set_option ($tc_conn_01, $parameters, 1);

		$userAfter = db2_get_option($tc_conn_01, "trusted_user");

		if($userBefore != $userAfter) {
			echo "User has been switched" . "\n";
			//Inserting into table using trusted_user.
			$sql_insert = "INSERT INTO $user.trusted_table (i1, i2) VALUES (?, ?)";
			$stmt = db2_prepare ($tc_conn_01, $sql_insert);
			$res = db2_execute($stmt, array(300, 500));

			//Updating table using trusted_user.
			$sql_update = "UPDATE $user.trusted_table set i1 = 400 WHERE i2 = 500";
			$stmt = db2_exec($tc_conn_01, $sql_update);
			echo db2_stmt_errormsg() . "\n";
		}
		else {
			echo "User is not switched" . "\n";
		}
	}

	db2_close($tc_conn_01);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn_01 = null;

/*
 * Creating 10 Persistance connections and checking if trusted context is enabled (Uncataloged connections)
 */

for($i = 1;$i <= 10;$i++) {
	$tc_conn = db2_pconnect($connString, "", "");
	if($tc_conn) {
		if(db2_get_option($tc_conn, "trustedcontext")) {
			if(db2_get_option($tc_conn, "trusted_user") == $userAfter) {
				echo "Explicit Trusted Connection succeeded.\n";
			}
		}
		db2_close($tc_conn);
	}
	else {
		echo "Explicit Trusted Connection failed.\n";
	}
}

/*
 * Setup when no trusted context is available.
 */

$conn_03 = db2_connect($database, $user, $password);
if ($conn_03) {
	echo "Connection succeeded. Displaying Content of trusted_table. Dropping Trusted Context.\n";

	//Printing table contents.
	print_table ($conn_03, "SELECT * FROM trusted_table");

	//Dropping Trusted Context and role.
	$res = @db2_exec($conn_03, $sql_drop_trusted_context);
	$res = @db2_exec($conn_03, $sql_drop_role);
	
	//Dropping table.
	$sql_drop_table = "DROP TABLE trusted_table";
	$res = @db2_exec($conn_03, $sql_drop_table);
	
	db2_close($conn_03);
}
else {
    echo "Connection failed.\n";
}

/*
 * Helping functions.
 */

//Creating trusted_table in database and populating.
function create_database ($conn) {
	//Dropping table.

	$sql_drop_table = "DROP TABLE trusted_table";
	$res = @db2_exec($conn, $sql_drop_table);
	
	//Creating table.
	$sql_create_table = "CREATE TABLE trusted_table(i1 int,i2 int)";
	$res = db2_exec($conn, $sql_create_table);
	
	//Pouplating table.
	$sql_insert = "INSERT INTO trusted_table (i1, i2) VALUES (?, ?)";
	$stmt = db2_prepare($conn, $sql_insert);
	for($i = 1;$i <= 2;$i++) {
		$res = db2_execute($stmt, array($i * 10, $i * 20));
	}
}

//Print the content of the table.
function print_table ($conn, $sql) {
	$stmt = db2_exec($conn, $sql);
	while(($row = db2_fetch_array($stmt)) != null) {
		var_dump($row);
	}
}

?>
--EXPECTF--
Connection succeeded. Creating new Trusted Context (WITH Authentication).
array(2) {
  [0]=>
  int(10)
  [1]=>
  int(20)
}
array(2) {
  [0]=>
  int(20)
  [1]=>
  int(40)
}
Explicit Trusted Connection succeeded.
Performing normal operations with trusted connection.
User has been switched

Warning: db2_exec(): Statement Execute Failed in %s on line %d
[%s][%s][%s] SQL0551N  %s does not have the privilege to perform operation "UPDATE" on object "%s.TRUSTED_TABLE".  SQLSTATE=42501 SQLCODE=-551
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Explicit Trusted Connection succeeded.
Connection succeeded. Displaying Content of trusted_table. Dropping Trusted Context.
array(2) {
  [0]=>
  int(10)
  [1]=>
  int(20)
}
array(2) {
  [0]=>
  int(20)
  [1]=>
  int(40)
}
array(2) {
  [0]=>
  int(300)
  [1]=>
  int(500)
}
