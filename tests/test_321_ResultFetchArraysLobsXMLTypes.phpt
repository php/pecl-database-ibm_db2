--TEST--
IBM-DB2: db2_result, db2_fetch_array tests lobs and xml types
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipif2.inc');
require_once('skipif3.inc');
?>
--FILE--
<?php
require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
	$sql = "values (xmlQuery('<x/>'), clob('a clob'), xmlserialize(xmlQuery('<x/>') as varchar(1000)))";
	$stmt = db2_prepare($conn, $sql);
	if (db2_execute($stmt)) {
		while (db2_fetch_row($stmt)) {
			for( $i = 0 ; $i < 3 ; $i++ ) {
				$x = db2_result($stmt,$i);
				echo "col $i:  type:", gettype($x), " ", db2_field_type($stmt,$i), " value:[", $x, "] false?", ($x===false),"\n";
			}
		}
	} else {
		print "execute failed\n";
	}

	if (db2_execute($stmt)) {
		while (($a = db2_fetch_array($stmt))) {
			echo "row: [$a[0]] [$a[1]] [$a[2]]\n";
		}
	} else {
		print "execute failed\n";
	}
}

?>
--XFAIL--
SQLGetData as a CLOB locator returns conversion error for some reason. Probably a bug, but it predates the CI
--EXPECT--
col 0:  type:string xml value:[<?xml version="1.0" encoding="UTF-8" ?><x/>] false?
col 1:  type:string clob value:[a clob] false?
col 2:  type:string string value:[<x/>] false?
row: [<?xml version="1.0" encoding="UTF-8" ?><x/>] [a clob] [<x/>]

