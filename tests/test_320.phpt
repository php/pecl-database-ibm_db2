--TEST--
IBM-DB2: db2_escape_string function 
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
	$orig = fopen("escape.dat", "rb");
	$new = fopen("escape_out.dat", "wb");

	$str[0] = "All characters: \x00 , \n , \r , \ , ' , \" , \x1a .";
	$str[1] = "Backslash (\). Single quote ('). Double quote (\")";
	$str[2] = "The NULL character \0 must be quoted as well";
	$str[3] = "Intersting characters: \x1a , \x00 .";
	$str[3] = "Nothing to quote";
	$str[4] = 200676;
	$str[5] = "";

	foreach( $str as $string ) {
        fwrite($new, "\n");
        fwrite($new, "Original:                 " . $string);
        fwrite($new, "\n");
        fwrite($new, "db2_escape_string:        " . db2_escape_string($string));
        fwrite($new, "\n");
	}

	$file0 = file_get_contents("escape.dat");
	$file1 = file_get_contents("escape_out.dat");

	if(strcmp($file0, $file1) == 0) {
		echo "The files are the same...good.";
	} else {
		echo "The files are not the same...bad.";
	}
}

?>
--EXPECT--
The files are the same...good.
