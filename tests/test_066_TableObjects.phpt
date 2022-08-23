--TEST--
IBM-DB2: db2_tables() - Play with table objects
--SKIPIF--
<?php require_once('skipif3.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$server = db2_server_info( $conn );

$result = @db2_tables($conn, NULL, strtoupper($user), 'ANIM%');

$t[0]=""; $t[1]=""; $t[2]=""; $t[3]="";
while ($row = db2_fetch_object($result)) {
	$output = 
    	'Schema:  ' . $row->TABLE_SCHEM . "\n"
    	. 'Name:    ' . $row->TABLE_NAME . "\n"
    	. 'Type:    ' . $row->TABLE_TYPE . "\n"
    	. 'Remarks: ' . $row->REMARKS . "\n\n";
	if ($server->DBMS_NAME == 'AS') {
		switch ($row->TABLE_NAME) {
			case 'ANIMALS':
				$t[0].=$output;
				break;
			case 'ANIMAL_PICS':
				$t[1].=$output;
				break;
			case 'ANIME_CAT':
				$t[2].=$output;
				break;
			default:
				$t[3].=$output;
				break;
		}
	} else {
    	echo $output;
	}
}

if ($server->DBMS_NAME == 'AS') {
	foreach ($t as $item) echo $item;
}

db2_free_result($result);
db2_close($conn);

?>
--EXPECTF--
Schema:  %s
Name:    ANIMALS
Type:    TABLE
Remarks: 

Schema:  %s
Name:    ANIMAL_PICS
Type:    TABLE
Remarks: 

Schema:  %s
Name:    ANIME_CAT
Type:    VIEW
Remarks: 

