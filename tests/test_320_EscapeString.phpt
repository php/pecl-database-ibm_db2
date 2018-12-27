--TEST--
IBM-DB2: db2_escape_string function 
--SKIPIF--
<?php require_once('skipif3.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
	$drop = 'DROP TABLE escapeit';
	$result = @db2_exec($conn, $drop);

	$create = 'CREATE TABLE escapeit(id INTEGER, info VARCHAR(200))';
	$result = @db2_exec($conn, $create);

	$str[0] = "Some random special characters: \n , \r , \ , ' , \"  .";
	$str[1] = "Backslash (\). Single quote ('). Double quote (\")";
	$str[2] = "The NULL character \\0 must be escaped manually";
	$str[3] = "Intersting characters must be escaped manually: \\x1a , \\x00 .";
	$str[4] = "Nothing to quote";
	$str[5] = 200676;
	$str[6] = "";

    $count = 0;
    $out = "";
	foreach( $str as $string ) {
        $escaped = db2_escape_string($string);
        $insert = "INSERT INTO escapeit VALUES($count, '$escaped')";
        db2_exec($conn, $insert);    

        $sql = "SELECT info FROM escapeit WHERE id = ?";
        $stmt = db2_prepare($conn, $sql);
        $currow = array($count);
        db2_execute($stmt, $currow);

        $result = db2_fetch_array($stmt);
        $escapedFromDb = $result[0];        

        $out .= "\n";
        $out .=  "Original:                 " . $string;
        $out .= "\n";
        $out .= "db2_escape_string:        " . $escapedFromDb;
        $out .= "\n";
        
        $count++;
	}

	$in = file_get_contents(dirname(__FILE__)."/escape.dat");

	if(strcmp($in, $out) == 0) {
		echo "The files are the same...good.\n";
	} else {
		echo "The files are not the same...bad.\n";
	}
	
	$name = "o'grady";
    $esc_name = db2_escape_string($name);

    $sql = "INSERT INTO escapeit(id, info) VALUES(100, 'o''grady')";
    db2_exec($conn, $sql);
    
    $select = "SELECT info FROM escapeit where info='$esc_name'";

    $stmt = db2_exec($conn, $select);
    while ($row = db2_fetch_array($stmt))
    {
        var_dump($row);
        if ($row[0] != $name) {
            "FAILED! " . $row[0] . " should equal $name";
        }
    }
}
?>
--EXPECT--
The files are the same...good.
array(1) {
  [0]=>
  string(7) "o'grady"
}
