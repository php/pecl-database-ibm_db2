--TEST--
IBM-DB2: db2_prepare / db2_execute: Checking for datatype conversions and errors when failed.
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
	//Drop the test table, in case it exists
	$dropTableSQL = "DROP TABLE TESTTABLE";
	$res = @db2_exec($conn, $dropTableSQL);

	//Create the test table
	$createTableSQL = "CREATE TABLE TESTTABLE (
		USERID INTEGER NOT NULL GENERATED ALWAYS AS IDENTITY (
			START WITH +0
			INCREMENT BY +1
			MINVALUE +0
			MAXVALUE +2147483647
			NO CYCLE
			NO CACHE
			NO ORDER
		),
		USERNAME VARCHAR(20),
		PASSWORD VARCHAR(20),
		NUMBER_SINT SMALLINT,
		NUMBER_INT INTEGER, 
		NUMBER_BINT BIGINT, 
		NUMBER_DOU DOUBLE, 
		NUMBER_R REAL, 
		NUMBER_DEC DECIMAL (7, 2)
	)";
	$res = db2_exec($conn, $createTableSQL);
	print db2_conn_error($conn);
	
	//Populating the test table
	$rows = array(
		array('user0',	'password0',	0,	0,	0,	0000.00,	0000.00,	0000.00),
		array('user1',	'password1',	1,	1,	1,	1000.00,	1000.00,	1000.00),
		array('user2',	'password2',	2,	2,	2,	2000.00,	2000.00,	2000.00),
		array('user3',	'password3',	3,	3,	3,	3000.00,	3000.00,	3000.00),
		array('user4',	'password4',	4,	4,	4,	4000.00,	4000.00,	4000.00),
		array('user5',	'password5',	5,	5,	5,	5000.00,	5000.00,	5000.00),
		array('user6',	'password6',	6,	6,	6,	6000.00,	6000.00,	6000.00),
		array('user7',	'password7',	7,	7,	7,	7000.00,	7000.00,	7000.00),
		array('user8',	'password8',	8,	8,	8,	8000.00,	8000.00,	8000.00),
		array('user9',	'password9',	9,	9,	9,	9000.00,	9000.00,	9000.00)
	);
	$insertIntoTableSQL = "INSERT INTO TESTTABLE (
		USERNAME, 
		PASSWORD, 
		NUMBER_SINT, 
		NUMBER_INT, 
		NUMBER_BINT, 
		NUMBER_DOU, 
		NUMBER_R, 
		NUMBER_DEC
	) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
	$sth = db2_prepare($conn, $insertIntoTableSQL);
	if($sth){
		foreach($rows as $row){
			$res = db2_execute($sth, $row);
		}
	}
	
	//Altering the test table.
	$alterTable = "ALTER TABLE TESTTABLE ADD PRIMARY KEY (USERID)";
	$res = @db2_exec($prepconn, $alterTable);

	$alterTable = "ALTER TABLE TESTTABLE ADD UNIQUE (USERNAME)";
	$res = @db2_exec($prepconn, $alterTable);

	$alterTable = "ALTER TABLE TESTTABLE ALTER COLUMN USERID RESTART WITH 10";
	$res = @db2_exec($prepconn, $alterTable);
	
	//Update query for Primary Key.
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE USERID = ?";
	
	//Updating row with fake USERID.
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('PK_pass0', 'fakeUserID');
	//print "Updating TESTTABLE with 'fake' USERID" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with USERID = '1' (PK Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('PK_pass1', '1');
	//print "Updating TESTTABLE with USERID = '1'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with USERID = 1 (PK Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('PK_pass2', 1);
	//print "Updating TESTTABLE with USERID = 1" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with USERID = '10' (PK Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('PK_pass3', '10');
	//print "Updating TESTTABLE with USERID = '10'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with USERID = 10 (PK Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('PK_pass4', 10);
	//print "Updating TESTTABLE with USERID = 10" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Update query for SMALLINT.
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_SINT = ?";
	
	//Updating row with fake NUMBER_SINT (SMALLINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('SINT_pass0', 'fakeNUMBER_SINT');
	//print "Updating TESTTABLE with 'fake' NUMBER_SINT" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_SINT = '1' (SMALLINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('SINT_pass1', '1');
	//print "Updating TESTTABLE with NUMBER_SINT = '1'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_SINT = 1 (SMALLINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('SINT_pass2', 1);
	//print "Updating TESTTABLE with NUMBER_SINT = 1" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_SINT = '10' (SMALLINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('SINT_pass3', '10');
	//print "Updating TESTTABLE with NUMBER_SINT = '10'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_SINT = 10 (SMALLINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('SINT_pass4', 10);
	//print "Updating TESTTABLE with NUMBER_SINT = 10" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";
	
	//Update query for INTEGER.
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_INT = ?";
	
	//Updating row with fake NUMBER_INT (INTEGER Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('INT_pass0', 'fakeNUMBER_INT');
	//print "Updating TESTTABLE with 'fake' NUMBER_INT" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_INT = '1' (INTEGER Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('INT_pass1', '1');
	//print "Updating TESTTABLE with NUMBER_INT = '1'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_INT = 1 (INTEGER Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('INT_pass2', 1);
	//print "Updating TESTTABLE with NUMBER_INT = 1" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_INT = '10' (INTEGER Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('INT_pass3', '10');
	//print "Updating TESTTABLE with NUMBER_INT = '10'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_INT = 10 (INTEGER Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('INT_pass4', 10);
	//print "Updating TESTTABLE with NUMBER_INT = 10" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";
	
	//Update query for BIGINT.
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_BINT = ?";
	
	//Updating row with fake NUMBER_BINT (BIGINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BINT_pass0', 'fakeNUMBER_BINT');
	//print "Updating TESTTABLE with 'fake' NUMBER_BINT" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_BINT = '1' (BIGINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BINT_pass1', '1');
	//print "Updating TESTTABLE with NUMBER_BINT = '1'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_BINT = 1 (BIGINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BINT_pass2', 1);
	//print "Updating TESTTABLE with NUMBER_BINT = 1" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_BINT = '10' (BIGINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BINT_pass3', '10');
	//print "Updating TESTTABLE with NUMBER_BINT = '10'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_BINT = 10 (BIGINT Test).
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BINT_pass4', 10);
	//print "Updating TESTTABLE with NUMBER_BINT = 10" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	
	//Updating row with fake NUMBER_DOU (DOUBLE Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DOU = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass0', 'fakeNUMBER_DOU');
	//print "Updating TESTTABLE with NUMBER_DOU = 'fake'" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DOU = '1000' (DOUBLE Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DOU = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass1', '1000');
	//print "Updating TESTTABLE with NUMBER_DOU = '1000'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DOU = 1000 (DOUBLE Test). 
	$updateRow = "UPDATE TESTTABLE SET NUMBER_DOU = ? WHERE NUMBER_DOU = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('1000.50', 1000);
	//print "Updating TESTTABLE with NUMBER_DOU = 1000" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DOU = '1000.50' (DOUBLE Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DOU = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass2', '1000.50');
	//print "Updating TESTTABLE with NUMBER_DOU = '1000.50'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DOU = 1000.50 (DOUBLE Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DOU = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass4', 1000.50);
	//print "Updating TESTTABLE with NUMBER_DOU = 1000.50" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";
	
	//Updating row with fake NUMBER_R (REAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_R = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass0', 'fakeNUMBER_R');
	//print "Updating TESTTABLE with NUMBER_R = 'fake'" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_R = '1000' (REAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_R = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass1', '1000');
	//print "Updating TESTTABLE with NUMBER_R = '1000'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_R = 1000 (REAL Test). 
	$updateRow = "UPDATE TESTTABLE SET NUMBER_R = ? WHERE NUMBER_R = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('1000.50', 1000);
	//print "Updating TESTTABLE with NUMBER_R = 1000" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_R = '1000.50' (REAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_R = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass2', '1000.50');
	//print "Updating TESTTABLE with NUMBER_R = '1000.50'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_R = 1000.50 (REAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_R = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass4', 1000.50);
	//print "Updating TESTTABLE with NUMBER_R = 1000.50" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";
	
	//Updating row with fake NUMBER_DEC (DECIMAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DEC = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass0', 'fakeNUMBER_DEC');
	//print "Updating TESTTABLE with NUMBER_DEC = 'fake'" . "\n";
	$result = @db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	else print @db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DEC = '1000' (DECIMAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DEC = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass1', '1000');
	//print "Updating TESTTABLE with NUMBER_DEC = '1000'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DEC = 1000 (DECIMAL Test). 
	$updateRow = "UPDATE TESTTABLE SET NUMBER_DEC = ? WHERE NUMBER_DEC = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('1000.50', 1000);
	//print "Updating TESTTABLE with NUMBER_DEC = 1000" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DEC = '1000.50' (DECIMAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DEC = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass2', '1000.50');
	//print "Updating TESTTABLE with NUMBER_DEC = '1000.50'" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with NUMBER_DEC = 1000.50 (DECIMAL Test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE NUMBER_DEC = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('DOU_pass4', 1000.50);
	//print "Updating TESTTABLE with NUMBER_DEC = 1000.50" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with USERID = TRUE (Boolean test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE USERID = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BOOL_pass_TRUE', TRUE);
	//print "Updating TESTTABLE with USERID = TRUE" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Updating row with USERID = FALSE (Boolean test).
	$updateRow = "UPDATE TESTTABLE SET PASSWORD = ? WHERE USERID = ?";
	$stmt = db2_prepare($conn , $updateRow);
	$parameters = array('BOOL_pass_FALSE', FALSE);
	//print "Updating TESTTABLE with USERID = FALSE" . "\n";
	$result = db2_execute($stmt, $parameters);
	if(!$result) {
		print "Test case failed." . "\n";
	}
	print db2_num_rows($stmt) . "\n";

	//Drop the test table, in case it exists
	$dropTableSQL = "DROP TABLE TESTTABLE";
	$res = @db2_exec($conn, $dropTableSQL);
	
	db2_close($conn);
}
else {
	print "Connection not created" . "\n";
}

?>
--EXPECTF--
%s
1
1
0
0
%s
1
1
0
0
%s
1
1
0
0
%s
1
1
0
0
%s
1
1
1
1
%s
1
1
1
1
%s
1
1
1
1
1
1

