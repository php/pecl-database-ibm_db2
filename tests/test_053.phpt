--TEST--
IBM-DB2: set autocommit with db2_autocommit
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
	require_once('connection.inc');

	$conn = db2_connect($database, $user, $password);

	echo "Client attributes passed through conection string:\n";

	$options1 = array('userid' => 'db2inst1');
	$conn1 = db2_connect($database, $user, $password, $options1);
	$val = db2_get_option($conn1, 'userid');
	echo $val . "\n";

	$options2 = array('acctstr' => 'account');
	$conn2 = db2_connect($database, $user, $password, $options2);
	$val = db2_get_option($conn2, 'acctstr');
	echo $val . "\n";

	$options3 = array('applname' => 'myapp');
	$conn3 = db2_connect($database, $user, $password, $options3);
	$val = db2_get_option($conn3, 'applname');
	echo $val . "\n";

	$options4 = array('wrkstnname' => 'workstation');
	$conn4 = db2_connect($database, $user, $password, $options4);
	$val = db2_get_option($conn4, 'wrkstnname');
	echo $val . "\n";

	echo "Client attributes passed post-conection:\n";

	$options5 = array('userid' => 'db2inst1');
	$conn5 = db2_connect($database, $user, $password);
	$rc = db2_set_option($conn5, $options5, 1);
	$val = db2_get_option($conn5, 'userid');
	echo $val . "\n";

	$options6 = array('acctstr' => 'account');
	$conn6 = db2_connect($database, $user, $password);
	$rc = db2_set_option($conn6, $options6, 1);
	$val = db2_get_option($conn6, 'acctstr');
	echo $val . "\n";

	$options7 = array('applname' => 'myapp');
	$conn7 = db2_connect($database, $user, $password);
	$rc = db2_set_option($conn7, $options7, 1);
	$val = db2_get_option($conn7, 'applname');
	echo $val . "\n";

	$options8 = array('wrkstnname' => 'workstation');
	$conn8 = db2_connect($database, $user, $password);
	$rc = db2_set_option($conn8, $options8, 1);
	$val = db2_get_option($conn8, 'wrkstnname');
	echo $val . "\n";
?>
--EXPECT--
Client attributes passed through conection string:
db2inst1
account
myapp
workstation
Client attributes passed post-conection:
db2inst1
account
myapp
workstation

