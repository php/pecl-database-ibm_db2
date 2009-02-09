--TEST--
IBM-DB2: Trusted Context with db2_connect
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

$connString = "DATABASE=$database;HOSTNAME=$hostname;PORT=$port;PROTOCOL=TCPIP;UID=$authID;PWD=$auth_pass;";
$tc_conn = db2_connect($connString, "", "");
if($tc_conn) {
	echo "Normal connection established.\n";
	$res = db2_set_option($tc_conn, array ("trustedcontext" => DB2_TRUSTED_CONTEXT_ENABLE), 1);
	
	$parameters = array("trusted_user" => $tcuser, "trusted_password" => $tcuser_pass);
	$res = db2_set_option ($tc_conn, $parameters, 1);

	db2_close($tc_conn);
}
else {
	echo "Connection failed.\n";
}
$tc_conn = null;

$tc_conn = db2_connect($connString, "", "");
if($tc_conn) {
	echo "Normal connection established.\n";
	$res = db2_set_option($tc_conn, array ("trustedcontext" => DB2_TRUSTED_CONTEXT_ENABLE, "trusted_user" => $tcuser, "trusted_password" => $tcuser_pass), 1);

	db2_close($tc_conn);
}
else {
	echo "Connection failed.\n";
}
$tc_conn = null;

$options = array ("trustedcontext" => DB2_TRUSTED_CONTEXT_ENABLE, "trusted_user" => $tcuser, "trusted_password" => $tcuser_pass);

$tc_conn = db2_connect($connString, "", "", $options);
if($tc_conn) {
	echo "Explicit Trusted Connection succeeded.\n";
	$trusted_user = db2_get_option($tc_conn, "trusted_user");
	if($trusted_user != $tcuser) {
		echo "But trusted user is not switched.\n";
	}

	db2_close($tc_conn);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn = null;

/* 
 * Create Trusted connection, switch with authentication and check for new user.
 */

$options = array ("trustedcontext" => DB2_TRUSTED_CONTEXT_ENABLE);

$tc_conn_01 = db2_connect($connString, "", "", $options);
if($tc_conn_01) {
	echo "Explicit Trusted Connection succeeded.\n";
	echo "Performing normal operations with trusted connection.\n";

	if(db2_get_option($tc_conn_01, "trustedcontext")) {
		$userBefore = db2_get_option($tc_conn_01, "trusted_user");

		//Switching to trusted user.
		$parameters = array("trusted_user" => $tcuser, "trusted_password" => $tcuser_pass);
		$res = db2_set_option ($tc_conn_01, $parameters, 1);

		$userAfter = db2_get_option($tc_conn_01, "trusted_user");

		if($userBefore != $userAfter) {
			echo "User has been switched." . "\n";
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
 * Playing with trusted context
 */

$tc_conn_02 = db2_connect($connString, "", "", $options);
if($tc_conn_02) {
	echo "Explicit Trusted Connection succeeded.\n";
	echo "Performing operations with fake user.\n";

	if(db2_get_option($tc_conn_02, "trustedcontext")) {
		$userBefore = db2_get_option($tc_conn_02, "trusted_user");

		//Switching to fake user.
		$res = db2_set_option ($tc_conn_02, array("trusted_user" => "FakeUser"), 1);
		$res = db2_set_option ($tc_conn_02, array("trusted_password" => "FakePassword"), 1);

		//Inserting into table using fakeuser.
		$sql_insert = "INSERT INTO $user.trusted_table (i1, i2) VALUES (?, ?)";
		$stmt = db2_prepare ($tc_conn_02, $sql_insert);
		$res = db2_execute($stmt, array(300, 500));
		echo db2_stmt_errormsg() . "\n";
	}

	db2_close($tc_conn_02);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn_02 = null;

$tc_conn_03 = db2_connect($connString, "", "", $options);
if($tc_conn_03) {
	echo "Explicit Trusted Connection succeeded.\n";
	echo "Performing operations when only user is set and not the password.\n";
	
	if(db2_get_option($tc_conn_03, "trustedcontext")) {
		$userBefore = db2_get_option($tc_conn_03, "trusted_user");

		//Switching to trusted user.
		$res = db2_set_option ($tc_conn_03, array("trusted_user" => $tcuser), 1);

		//Inserting into table using trusted_user.
		$sql_insert = "INSERT INTO $user.trusted_table (i1, i2) VALUES (300, 500)";
		$res = db2_exec ($tc_conn_03, $sql_insert);
		echo db2_stmt_errormsg() . "\n";
	}
	
	db2_close($tc_conn_03);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn_03 = null;

$tc_conn_04 = db2_connect($connString, "", "", $options);
if($tc_conn_04) {
	echo "Explicit Trusted Connection succeeded.\n";
	echo "Performing operations when order of username and password has been altered while setting it in connection.\n";
	
	if(db2_get_option($tc_conn_04, "trustedcontext")) {
		$userBefore = db2_get_option($tc_conn_04, "trusted_user");

		//Setting password first.
		$res = db2_set_option ($tc_conn_04, array("trusted_password" => $tcuser_pass), 1);
		$res = db2_set_option ($tc_conn_04, array("trusted_password" => $tcuser), 1);

		$userAfter = db2_get_option($tc_conn_04, "trusted_user");
		if ($userBefore != $userAfter) {
			echo "User has been switched.\n";
		}
		else {
			echo db2_conn_errormsg() . "\n";
		}
	}
	
	db2_close($tc_conn_04);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn_04 = null;

$tc_conn_05 = db2_connect($connString, "", "", $options);
if($tc_conn_05) {
	echo "Explicit Trusted Connection succeeded.\n";
	echo "Performing operations when order of username and password has been altered while setting it in connection.\n";
	
	if(db2_get_option($tc_conn_05, "trustedcontext")) {
		$userBefore = db2_get_option($tc_conn_05, "trusted_user");

		//Switching to trusted user.
		$res = db2_set_option ($tc_conn_05, array("trusted_password" => $tcuser_pass, "trusted_user" => $tcuser), 1);

		$userAfter = db2_get_option($tc_conn_05, "trusted_user");
		if ($userBefore != $userAfter) {
			echo "User has been switched.\n";
		}
	}

	db2_close($tc_conn_05);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn_05 = null;

/*
 * Creating set up for trusted context WITHOUT AUTHENTICATION
 */

$conn_02 = db2_connect($database, $user, $password);

if ($conn_02) {
	echo "Connection succeeded. Creating new Trusted Context (WITHOUT Authentication).\n";

	//Dropping Trusted Context.	
	$res = @db2_exec($conn_02, $sql_drop_trusted_context);

	//Dropping Role.	
	$res = @db2_exec($conn_02, $sql_drop_role);
	
	//Creating Role.	
	$res = @db2_exec($conn_02, $sql_create_role);
	
	//Granting permissions to role.
	$sql_grant_permission = "GRANT UPDATE ON TABLE trusted_table TO ROLE role_01";
	$res = @db2_exec($conn_02, $sql_grant_permission);
	
	//Creating Trusted Context for user with authentication.
	$sql_create_trusted_context_01 = $sql_create_trusted_context . " WITHOUT AUTHENTICATION";
	$res = db2_exec($conn_02, $sql_create_trusted_context_01);
		
	db2_close($conn_02);
}
else {
	echo db2_conn_errormsg() . "\n";
	echo "Connection failed.\n";
}
$conn_02 = null;

/* 
 * Create Trusted connection, switch without authentication and check for new user.
 */

$tc_conn_06 = db2_connect($connString, "", "", $options);
if($tc_conn_06) {
	echo "Explicit Trusted Connection succeeded.\n";
	
	if(db2_get_option($tc_conn_06, "trustedcontext")) {
		$userBefore = db2_get_option($tc_conn_06, "trusted_user");

		//Switching to trusted user.
		$parameters = array("trusted_user" => $tcuser);
		$res = db2_set_option ($tc_conn_06, $parameters, 1);
		$userAfter = db2_get_option($tc_conn_06, "trusted_user");

		if($userBefore != $userAfter) {
			//Inserting into table using trusted_user.
			$sql_insert = "INSERT INTO $user.trusted_table (i1, i2) VALUES (1000, 1500)";
			$res = db2_exec($tc_conn_06, $sql_insert);
			echo db2_stmt_errormsg() . "\n";

			//Updating table using trusted_user.
			$sql_update = "UPDATE $user.trusted_table set i1 = 2000 WHERE i2 = 20";
			$res = db2_exec($tc_conn_06, $sql_update);
		}
	}

	db2_close($tc_conn_06);
}
else {
	echo "Explicit Trusted Connection failed.\n";
}
$tc_conn_06 = null;

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
Normal connection established.

Warning: db2_set_option(): TRUSTED CONTEXT connection attribute can only be set on connection resources in %s on line 56

Warning: db2_set_option(): TRUSTED USER attribute can only be set when TRUSTED CONTEXT is enabled in %s on line 59

Warning: db2_set_option(): TRUSTED PASSWORD attribute can only be set when TRUSTED CONTEXT is enabled in %s on line 59
Normal connection established.

Warning: db2_set_option(): TRUSTED CONTEXT connection attribute can only be set on connection resources in %s on line 71

Warning: db2_set_option(): TRUSTED USER attribute can only be set when TRUSTED CONTEXT is enabled in %s on line 71

Warning: db2_set_option(): TRUSTED PASSWORD attribute can only be set when TRUSTED CONTEXT is enabled in %s on line 71

Warning: db2_connect(): TRUSTED USER attribute can only be set on live connection in %s on line 82

Warning: db2_connect(): TRUSTED PASSWORD attribute can only be set on live connection in %s on line 82
Explicit Trusted Connection succeeded.
But trusted user is not switched.
Explicit Trusted Connection succeeded.
Performing normal operations with trusted connection.
User has been switched.

Warning: db2_exec(): Statement Execute Failed in %s on line 126
[%s][%s][%s] SQL0551N  %s does not have the privilege to perform operation "UPDATE" on object "%s.TRUSTED_TABLE".  SQLSTATE=42501 SQLCODE=-551
Explicit Trusted Connection succeeded.
Performing operations with fake user.

Warning: db2_execute(): Describe Param Failed in %s on line 160

Warning: db2_execute(): Binding Error in %s on line 160
[%s][%s][%s] SQL30082N  Security processing failed with reason "24" ("USERNAME AND/OR PASSWORD INVALID").  SQLSTATE=08001 SQLCODE=-30082
Explicit Trusted Connection succeeded.
Performing operations when only user is set and not the password.

Warning: db2_exec(): Statement Execute Failed in %s on line 184
[%s][%s][%s] SQL20361N  The switch user request using authorization ID %s within trusted context "CTX" failed with reason code "2".  SQLSTATE=42517 SQLCODE=-20361
Explicit Trusted Connection succeeded.
Performing operations when order of username and password has been altered while setting it in connection.
[%s][%s] CLI0198E  Missing trusted context userid. SQLSTATE=HY010 SQLCODE=-99999
Explicit Trusted Connection succeeded.
Performing operations when order of username and password has been altered while setting it in connection.
User has been switched.
Connection succeeded. Creating new Trusted Context (WITHOUT Authentication).
Explicit Trusted Connection succeeded.

Warning: db2_exec(): Statement Execute Failed in %s on line 300
[%s][%s][%s] SQL0551N  "%s" does not have the privilege to perform operation "INSERT" on object "%s.TRUSTED_TABLE".  SQLSTATE=42501 SQLCODE=-551
Connection succeeded. Displaying Content of trusted_table. Dropping Trusted Context.
array(2) {
  [0]=>
  int(2000)
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
