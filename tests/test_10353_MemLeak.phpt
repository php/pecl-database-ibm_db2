--TEST--
IBM-DB2: PECL bug 10353 -- Memory leak testing
--SKIPIF--
<?php
  require_once('skipif.inc');
  if(version_compare(PHP_VERSION, '7.0.0', '<') == 1) die("skip: Test segfaults on PHP 5.6");
  if(ZEND_DEBUG_BUILD == false) die("skip: test is allegedly pointless on release builds");
?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

echo "Test requires PHP to be compiled in debug mode\n";

/* Testing db2_prepare leaks */
$sql = "FAULTY SQL SHOULD NOT LEAK MEMORY";
$stmt = db2_prepare($conn, $sql, array('deferred_prepare' => DB2_DEFERRED_PREPARE_OFF));
if ($stmt) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_exec leaks */
$sql = "FAULTY SQL SHOULD NOT LEAK MEMORY";
$stmt = db2_exec($conn, $sql);
if ($stmt) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_tables leaks */
$stmt = db2_tables($conn, NULL, NULL, NULL);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_table_privileges leaks */
$stmt = db2_table_privileges($conn, NULL, NULL, NULL);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_statistics leaks */
$stmt = db2_statistics($conn, NULL, NULL, NULL, 1);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_special_columns leaks */
$stmt = db2_special_columns($conn, NULL, NULL, NULL, 0);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_procedures leaks */
$stmt = db2_procedures($conn, NULL, NULL, NULL);
$row = db2_fetch_assoc($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_procedure_columns leaks */
$stmt = db2_procedure_columns($conn, NULL, NULL, NULL, NULL);
$row = db2_fetch_assoc($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_primary_keys leaks */
$stmt = db2_primary_keys($conn, NULL, NULL, NULL);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_foreign_keys leaks */
$stmt = db2_foreign_keys($conn, NULL, NULL, NULL);
/* XXX: This will fail with a TypeError in PHP 8.x instead */
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_column_privileges leaks */
$stmt = db2_column_privileges($conn, NULL, NULL, NULL);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks\n";
}

/* Testing db2_columns leaks */
$stmt = db2_columns($conn, NULL, NULL, NULL);
$row = db2_fetch_array($stmt);
if ($row) {
	echo "Shouldn't be here\n";
} else {
	echo "Good, no memory leaks";
}

?>
--EXPECTF--
Test requires PHP to be compiled in debug mode

Warning: db2_prepare(): Statement Prepare Failed in %s
Good, no memory leaks

Warning: db2_exec(): Statement Execute Failed in %s
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks

Warning: db2_fetch_array() expects parameter 1 to be resource, bool%S given in %s
Good, no memory leaks
Good, no memory leaks
Good, no memory leaks
