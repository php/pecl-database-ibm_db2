--TEST--
IBM-DB2: Test for XML OUT parameters in stored procedures. 
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

    $dbconn = db2_connect('manas', 'manas', 'Pswd4QAT');

    $stmt = db2_prepare($dbconn, 'DROP PROCEDURE test_xml1');
    $res = @db2_execute($stmt);
    db2_free_stmt($stmt);

$create = <<<ENDPROC
  CREATE PROCEDURE test_xml1 (INOUT in_out CLOB,
    OUT output VARCHAR(1024), OUT output2 XML)
  DYNAMIC RESULT SETS 0 READS SQL DATA LANGUAGE SQL NO EXTERNAL ACTION
  BEGIN
    SET in_out = XMLSERIALIZE(
      XMLELEMENT (NAME "test", XMLATTRIBUTES ('value' AS "attribute"))
      AS CLOB);
    SET output = XMLSERIALIZE(
      XMLELEMENT (NAME "test", XMLATTRIBUTES ('value' AS "attribute"))
      AS VARCHAR(1024));
    SET output2 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));
  END
ENDPROC;

    $stmt = db2_prepare($dbconn, $create);
    $res = db2_execute($stmt);
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'DROP PROCEDURE test_xml2');
    $res = @db2_execute($stmt);
    db2_free_stmt($stmt);

$create = <<<ENDPROC
  CREATE PROCEDURE test_xml2(OUT output1 XML, OUT output2 XML,
                              OUT output3 XML, OUT output4 XML,
                                  OUT output5 XML, OUT output6 XML)
  DYNAMIC RESULT SETS 0 READS SQL DATA LANGUAGE SQL NO EXTERNAL ACTION
  BEGIN
    SET output1 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));
    SET output2 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));
    SET output3 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));
    SET output4 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));
    SET output5 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));
    SET output6 =
      XMLELEMENT(NAME "test", XMLATTRIBUTES('value' AS "attribute"));

  END 
ENDPROC;

    $stmt = db2_prepare($dbconn, $create);
    $res = db2_execute($stmt);
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'DROP PROCEDURE test_xml3');
    $res = @db2_execute($stmt);
    db2_free_stmt($stmt);

$create = <<<ENDPROC
  CREATE PROCEDURE test_xml3(INOUT in_out XML, IN input XML, OUT output XML)
  DYNAMIC RESULT SETS 0 READS SQL DATA LANGUAGE SQL NO EXTERNAL ACTION
  BEGIN
    SET output = in_out;
    SET in_out = input;
  END 
ENDPROC;

    $stmt = db2_prepare($dbconn, $create);
    $res = db2_execute($stmt);
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'CALL test_xml1(?, ?, ?)');
    $clobResult = 'here are some characters';
    $charResult = '';
    $xmlResult = '';
    db2_bind_param($stmt, 1, "clobResult", DB2_PARAM_INOUT, DB2_CHAR);
    db2_bind_param($stmt, 2, "charResult", DB2_PARAM_OUT, DB2_CHAR);
    db2_bind_param($stmt, 3, "xmlResult", DB2_PARAM_OUT, DB2_XML);
    $res = db2_execute($stmt);
    if (!$res) {
        echo db2_stmt_errormsg();
    }
    echo 'clobResult: ' . $clobResult . "\n";
    echo 'charResult: ' . $charResult . "\n";
    echo 'xmlResult: ' . $xmlResult . "\n";
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'CALL test_xml2(?, ?, ?, ?, ?, ?)');
    $xmlResult1 = '';
    $xmlResult2 = ' ';
    $xmlResult3 = '                                                               ';
    $xmlResult4 = '                                                                ';
    $xmlResult5 = '                                                                 ';
    $xmlResult6 = '                                                                  ';

    db2_bind_param($stmt, 1, "xmlResult1", DB2_PARAM_OUT, DB2_XML);
    db2_bind_param($stmt, 2, "xmlResult2", DB2_PARAM_OUT, DB2_XML);
    db2_bind_param($stmt, 3, "xmlResult3", DB2_PARAM_OUT, DB2_XML);
    db2_bind_param($stmt, 4, "xmlResult4", DB2_PARAM_OUT, DB2_XML);
    db2_bind_param($stmt, 5, "xmlResult5", DB2_PARAM_OUT, DB2_XML);
    db2_bind_param($stmt, 6, "xmlResult6", DB2_PARAM_OUT, DB2_XML);
    $res = db2_execute($stmt);
    if (!$res) {
        echo db2_stmt_errormsg();
    }
    echo 'xmlResult1: ' . $xmlResult1 . "\n";
    echo 'xmlResult2: ' . $xmlResult2 . "\n";
    echo 'xmlResult3: ' . $xmlResult3 . "\n";
    echo 'xmlResult4: ' . $xmlResult4 . "\n";
    echo 'xmlResult5: ' . $xmlResult5 . "\n";
    echo 'xmlResult6: ' . $xmlResult6 . "\n";
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'CALL test_xml3(?, ?, ?)');
    $xmlResult1 = '<?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>';
    $xmlIn = '<?xml version="1.0" encoding="UTF-8" ?><xml attribute="test"/>';
    $xmlResult2 = '';
    db2_bind_param($stmt, 1, "xmlResult1", DB2_PARAM_INOUT, DB2_XML);
    db2_bind_param($stmt, 2, "xmlIn", DB2_PARAM_IN, DB2_XML);
    db2_bind_param($stmt, 3, "xmlResult2", DB2_PARAM_OUT, DB2_XML);

    $res = db2_execute($stmt);
    if (!$res) {
        echo db2_stmt_errormsg();
    }
    echo 'xmlResult1: ' . $xmlResult1 . "\n";
    echo 'xmlIn: ' . $xmlIn . "\n";
    echo 'xmlResult2: ' . $xmlResult2 . "\n";
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'CALL test_xml2(?, ?, ?, ?, ?, ?)');
    $xmlResult1 = '';
    $xmlResult2 = '';
    $xmlResult3 = '';
    $xmlResult4 = '';
    $xmlResult5 = '';
    $xmlResult6 = '';

    db2_bind_param($stmt, 1, "xmlResult1", DB2_PARAM_IN, DB2_XML, 2);
    db2_bind_param($stmt, 2, "xmlResult2", DB2_PARAM_IN, DB2_XML, 5, 3);
    db2_bind_param($stmt, 3, "xmlResult3", DB2_PARAM_OUT, DB2_XML, 5);
    db2_bind_param($stmt, 4, "xmlResult4", DB2_PARAM_OUT, DB2_XML, -1, 10);
    db2_bind_param($stmt, 5, "xmlResult5", DB2_PARAM_INOUT, DB2_XML, 0, 4);
    db2_bind_param($stmt, 6, "xmlResult6", DB2_PARAM_INOUT, DB2_XML, 2, 4);
    $res = db2_execute($stmt);
    if (!$res) {
        echo db2_stmt_errormsg();
    }
    echo 'xmlResult1: ' . $xmlResult1 . "\n";
    echo 'xmlResult2: ' . $xmlResult2 . "\n";
    echo 'xmlResult3: ' . $xmlResult3 . "\n";
    echo 'xmlResult4: ' . $xmlResult4 . "\n";
    echo 'xmlResult5: ' . $xmlResult5 . "\n";
    echo 'xmlResult6: ' . $xmlResult6 . "\n";
    db2_free_stmt($stmt);

    $stmt = db2_prepare($dbconn, 'CALL test_xml1(?, ?, ?)');
    $clobResult = 'here are some characters';
    $charResult = '';
    $xmlResult = '';
    db2_bind_param($stmt, 1, "clobResult", DB2_PARAM_IN, DB2_CHAR, 2);
    db2_bind_param($stmt, 2, "charResult", DB2_PARAM_OUT, DB2_CHAR, 1, 10);
    db2_bind_param($stmt, 3, "xmlResult", DB2_PARAM_OUT, DB2_XML, 0);
    $res = db2_execute($stmt);
    if (!$res) {
        echo db2_stmt_errormsg();
    }
    echo 'clobResult: ' . $clobResult . "\n";
    echo 'charResult: ' . $charResult . "\n";
    echo 'xmlResult: ' . $xmlResult . "\n";
    db2_free_stmt($stmt);

?>
--EXPECT--
clobResult: <test attribute="value"/>
charResult: <test attribute="value"/>
xmlResult: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult1: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult2: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult3: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult4: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult5: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult6: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult1: <?xml version="1.0" encoding="UTF-8" ?><xml attribute="test"/>
xmlIn: <?xml version="1.0" encoding="UTF-8" ?><xml attribute="test"/>
xmlResult2: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult1: 
xmlResult2: 
xmlResult3: <?xml
xmlResult4: <?xml version="1.0" encoding="UTF-8" ?><test attribute="value"/>
xmlResult5: 
xmlResult6: <?
clobResult: here are some characters
charResult: <
xmlResult: 
