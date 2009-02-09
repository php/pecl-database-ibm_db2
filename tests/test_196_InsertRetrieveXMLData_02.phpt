--TEST--
IBM-DB2: insert and retrieve XML data: 2
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
	$rc = @db2_exec($conn, "DROP TABLE xml_test");
	$rc = db2_exec($conn, "CREATE TABLE xml_test (id INTEGER, data VARCHAR(50), xmlcol XML)");
	$rc = db2_exec($conn, "INSERT INTO xml_test (id, data, xmlcol) values (1, 'xml test 1', '<address><street>12485 S Pine St.</street><city>Olathe</city><state>KS</state><zip>66061</zip></address>')");

	$sql = "SELECT * FROM xml_test";
	$stmt = db2_prepare($conn, $sql);
	db2_execute($stmt);
	while($result = db2_fetch_both($stmt)) {
		echo "Result ID: ";
		echo $result[0] . "\n";
		echo "Result DATA: ";
		echo $result[1] . "\n";
		echo "Result XMLCOL: ";
		echo $result[2] . "\n";
	}

	$sql = "SELECT XMLSERIALIZE(XMLQUERY('for \$i in \$t/address where \$i/city = \"Olathe\" return <zip>{\$i/zip/text()}</zip>' passing c.xmlcol as \"t\") AS CLOB(32k)) FROM xml_test c WHERE id = 1";
	$stmt = db2_prepare($conn, $sql);
	db2_execute($stmt);
	while($result = db2_fetch_both($stmt)) {
		echo "Result from XMLSerialize and XMLQuery: ";
		echo $result[0] . "\n";
	}

	$sql = "select xmlquery('for \$i in \$t/address where \$i/city = \"Olathe\" return <zip>{\$i/zip/text()}</zip>' passing c.xmlcol as \"t\") from xml_test c where id = 1";
	$stmt = db2_prepare($conn, $sql);
	db2_execute($stmt);
	while($result = db2_fetch_both($stmt)) {
		echo "Result from only XMLQuery: ";
		echo $result[0] . "\n";
	}

} else {
	echo 'no connection: ' + db2_conn_errormsg();
}

?>
--EXPECTF--
Result ID: 1
Result DATA: xml test 1
Result XMLCOL:%s<address><street>12485 S Pine St.</street><city>Olathe</city><state>KS</state><zip>66061</zip></address>
Result from XMLSerialize and XMLQuery: <zip>66061</zip>
Result from only XMLQuery:%s<zip>66061</zip>

