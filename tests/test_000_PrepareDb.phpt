--TEST--
IBM-DB2: prepare the database
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');
$conn = db2_connect($database, $user, $password);

if ($conn) {
    //Drop the animal table, in case it exists
    $drop = 'DROP TABLE ANIMALS';
    $res = @db2_exec($conn, $drop);

    //Create the animal table
    $create = 'CREATE TABLE ANIMALS (ID INTEGER, BREED VARCHAR(32), NAME CHAR(16), WEIGHT DECIMAL(7,2))';
    $res = db2_exec($conn, $create);

    //Populate the animal table
    $animals = array(
      array(0, 'cat',        'Pook',         3.2),
      array(1, 'dog',        'Peaches',      12.3),
      array(2, 'horse',      'Smarty',       350.0),
      array(3, 'gold fish',  'Bubbles',      0.1),
      array(4, 'budgerigar', 'Gizmo',        0.2),
      array(5, 'goat',       'Rickety Ride', 9.7),
      array(6, 'llama',      'Sweater',      150)
    );
    $insert = 'INSERT INTO ANIMALS (ID, BREED, NAME, WEIGHT) VALUES (?, ?, ?, ?)';
    $sth = db2_prepare($conn, $insert);
    if($sth){
       foreach($animals as $row){
          $res = db2_execute($sth, $row);
       }
    }

    //Drop the anime_cat view, in case it exists
    $drop = 'DROP VIEW ANIME_CAT';
    $res = @db2_exec($conn, $drop);

    //Create the anime_cat view
    $create = 'CREATE VIEW ANIME_CAT AS SELECT NAME, BREED FROM ANIMALS WHERE ID=0';
    $res = db2_exec($conn, $create);

    //Drop the animal_pics table
    $drop = 'DROP TABLE ANIMAL_PICS';
    $res = @db2_exec($conn, $drop);

    //Create the animal_pics table
    $create = 'CREATE TABLE ANIMAL_PICS (NAME VARCHAR(32), PICTURE BLOB(16K))';
    $res = db2_exec($conn, $create);

    //Populate the view table
    $animals = array(
        array('Spook', 'spook.png'),
        array('Helmut', 'pic1.jpg')
    );

    $insert = 'INSERT INTO ANIMAL_PICS (NAME, PICTURE) VALUES (?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if(!$stmt){
       echo "Attempt to prepare statement failed.";
       return 0;
    }

    foreach($animals as $row){
       $name = $row[0];
       $picname = $row[1];
       $picture = dirname(__FILE__) . "/$picname";
       db2_bind_param($stmt, 1, "name", DB2_PARAM_IN);
       db2_bind_param($stmt, 2, "picture", DB2_PARAM_FILE, DB2_BINARY);
       $res = db2_execute($stmt);
    }

    // Drop the department table, in case it exists
    $drop = 'DROP TABLE DEPARTMENT';
    $res = @db2_exec($conn, $drop);

    //Create the department table
    $create = 'CREATE TABLE DEPARTMENT (DEPTNO CHAR(3) NOT NULL, DEPTNAME VARCHAR(29) NOT NULL, MGRNO CHAR(6), ADMRDEPT CHAR(3) NOT NULL, LOCATION CHAR(16))';
    $res = db2_exec($conn, $create);

    // Populate the department table
    $department = array(
      array('A00', 'SPIFFY COMPUTER SERVICE DIV.', '000010', 'A00', null),
      array('B01', 'PLANNING',                     '000020', 'A00', null),
      array('C01', 'INFORMATION CENTER',           '000030', 'A00', null),
      array('D01', 'DEVELOPMENT CENTER',            null,     'A00', null),
      array('D11', 'MANUFACTURING SYSTEMS',        '000060', 'D01', null),
      array('D21', 'ADMINISTRATION SYSTEMS',       '000070', 'D01', null),
      array('E01', 'SUPPORT SERVICES',             '000050', 'A00', null),
      array('E11', 'OPERATIONS',                   '000090', 'E01', null),
      array('E21', 'SOFTWARE SUPPORT',             '000100', 'E01', null)
    );

    $insert = 'INSERT INTO DEPARTMENT (DEPTNO, DEPTNAME, MGRNO, ADMRDEPT, LOCATION) VALUES (?, ?, ?, ?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach($department as $row){
           $res = db2_execute($stmt, $row);
        }
    }

    // Drop the emp_act table, in case it exists
    $drop = 'DROP TABLE EMP_ACT';
    $res = @db2_exec($conn, $drop);

    //Create the emp_act table
    $create = 'CREATE TABLE EMP_ACT (EMPNO CHAR(6) NOT NULL, PROJNO CHAR(6) NOT NULL, ACTNO SMALLINT NOT NULL, EMPTIME DECIMAL(5,2), EMSTDATE DATE, EMENDATE DATE)';
    $res = db2_exec($conn, $create);

    // Populate the emp_act table
    $emp_act = array(
      array('000010', 'MA2100',   10,   0.50,  '1982-01-01',  '1982-11-01'),
      array('000010', 'MA2110',   10,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000010', 'AD3100',   10,   0.50,  '1982-01-01',  '1982-07-01'),
      array('000020', 'PL2100',   30,   1.00,  '1982-01-01',  '1982-09-15'),
      array('000030', 'IF1000',   10,   0.50,  '1982-06-01',  '1983-01-01'),
      array('000030', 'IF2000',   10,   0.50,  '1982-01-01',  '1983-01-01'),
      array('000050', 'OP1000',   10,   0.25,  '1982-01-01',  '1983-02-01'),
      array('000050', 'OP2010',   10,   0.75,  '1982-01-01',  '1983-02-01'),
      array('000070', 'AD3110',   10,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000090', 'OP1010',   10,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000100', 'OP2010',   10,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000110', 'MA2100',   20,   1.00,  '1982-01-01',  '1982-03-01'),
      array('000130', 'IF1000',   90,   1.00,  '1982-01-01',  '1982-10-01'),
      array('000130', 'IF1000',  100,   0.50,  '1982-10-01',  '1983-01-01'),
      array('000140', 'IF1000',   90,   0.50,  '1982-10-01',  '1983-01-01'),
      array('000140', 'IF2000',  100,   1.00,  '1982-01-01',  '1982-03-01'),
      array('000140', 'IF2000',  100,   0.50,  '1982-03-01',  '1982-07-01'),
      array('000140', 'IF2000',  110,   0.50,  '1982-03-01',  '1982-07-01'),
      array('000140', 'IF2000',  110,   0.50,  '1982-10-01',  '1983-01-01'),
      array('000150', 'MA2112',   60,   1.00,  '1982-01-01',  '1982-07-15'),
      array('000150', 'MA2112',  180,   1.00,  '1982-07-15',  '1983-02-01'),
      array('000160', 'MA2113',   60,   1.00,  '1982-07-15',  '1983-02-01'),
      array('000170', 'MA2112',   60,   1.00,  '1982-01-01',  '1983-06-01'),
      array('000170', 'MA2112',   70,   1.00,  '1982-06-01',  '1983-02-01'),
      array('000170', 'MA2113',   80,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000180', 'MA2113',   70,   1.00,  '1982-04-01',  '1982-06-15'),
      array('000190', 'MA2112',   70,   1.00,  '1982-02-01',  '1982-10-01'),
      array('000190', 'MA2112',   80,   1.00,  '1982-10-01',  '1983-10-01'),
      array('000200', 'MA2111',   50,   1.00,  '1982-01-01',  '1982-06-15'),
      array('000200', 'MA2111',   60,   1.00,  '1982-06-15',  '1983-02-01'),
      array('000210', 'MA2113',   80,   0.50,  '1982-10-01',  '1983-02-01'),
      array('000210', 'MA2113',  180,   0.50,  '1982-10-01',  '1983-02-01'),
      array('000220', 'MA2111',   40,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000230', 'AD3111',   60,   1.00,  '1982-01-01',  '1982-03-15'),
      array('000230', 'AD3111',   60,   0.50,  '1982-03-15',  '1982-04-15'),
      array('000230', 'AD3111',   70,   0.50,  '1982-03-15',  '1982-10-15'),
      array('000230', 'AD3111',   80,   0.50,  '1982-04-15',  '1982-10-15'),
      array('000230', 'AD3111',  180,   1.00,  '1982-10-15',  '1983-01-01'),
      array('000240', 'AD3111',   70,   1.00,  '1982-02-15',  '1982-09-15'),
      array('000240', 'AD3111',   80,   1.00,  '1982-09-15',  '1983-01-01'),
      array('000250', 'AD3112',   60,   1.00,  '1982-01-01',  '1982-02-01'),
      array('000250', 'AD3112',   60,   0.50,  '1982-02-01',  '1982-03-15'),
      array('000250', 'AD3112',   60,   0.50,  '1982-12-01',  '1983-01-01'),
      array('000250', 'AD3112',   60,   1.00,  '1983-01-01',  '1983-02-01'),
      array('000250', 'AD3112',   70,   0.50,  '1982-02-01',  '1982-03-15'),
      array('000250', 'AD3112',   70,   1.00,  '1982-03-15',  '1982-08-15'),
      array('000250', 'AD3112',   70,   0.25,  '1982-08-15',  '1982-10-15'),
      array('000250', 'AD3112',   80,   0.25,  '1982-08-15',  '1982-10-15'),
      array('000250', 'AD3112',   80,   0.50,  '1982-10-15',  '1982-12-01'),
      array('000250', 'AD3112',  180,   0.50,  '1982-08-15',  '1983-01-01'),
      array('000260', 'AD3113',   70,   0.50,  '1982-06-15',  '1982-07-01'),
      array('000260', 'AD3113',   70,   1.00,  '1982-07-01',  '1983-02-01'),
      array('000260', 'AD3113',   80,   1.00,  '1982-01-01',  '1982-03-01'),
      array('000260', 'AD3113',   80,   0.50,  '1982-03-01',  '1982-04-15'),
      array('000260', 'AD3113',  180,   0.50,  '1982-03-01',  '1982-04-15'),
      array('000260', 'AD3113',  180,   1.00,  '1982-04-15',  '1982-06-01'),
      array('000260', 'AD3113',  180,   0.50,  '1982-06-01',  '1982-07-01'),
      array('000270', 'AD3113',   60,   0.50,  '1982-03-01',  '1982-04-01'),
      array('000270', 'AD3113',   60,   1.00,  '1982-04-01',  '1982-09-01'),
      array('000270', 'AD3113',   60,   0.25,  '1982-09-01',  '1982-10-15'),
      array('000270', 'AD3113',   70,   0.75,  '1982-09-01',  '1982-10-15'),
      array('000270', 'AD3113',   70,   1.00,  '1982-10-15',  '1983-02-01'),
      array('000270', 'AD3113',   80,   1.00,  '1982-01-01',  '1982-03-01'),
      array('000270', 'AD3113',   80,   0.50,  '1982-03-01',  '1982-04-01'),
      array('000280', 'OP1010',  130,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000290', 'OP1010',  130,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000300', 'OP1010',  130,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000310', 'OP1010',  130,   1.00,  '1982-01-01',  '1983-02-01'),
      array('000320', 'OP2011',  140,   0.75,  '1982-01-01',  '1983-02-01'),
      array('000320', 'OP2011',  150,   0.25,  '1982-01-01',  '1983-02-01'),
      array('000330', 'OP2012',  140,   0.25,  '1982-01-01',  '1983-02-01'),
      array('000330', 'OP2012',  160,   0.75,  '1982-01-01',  '1983-02-01'),
      array('000340', 'OP2013',  140,   0.50,  '1982-01-01',  '1983-02-01'),
      array('000340', 'OP2013',  170,   0.50,  '1982-01-01',  '1983-02-01'),
      array('000020', 'PL2100',   30,   1.00,  '1982-01-01',  '1982-09-15')
    );

    $insert = 'INSERT INTO EMP_ACT (EMPNO, PROJNO, ACTNO, EMPTIME, EMSTDATE, EMENDATE) VALUES (?, ?, ?, ?, ?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach($emp_act as $row){
           $res = db2_execute($stmt, $row);
        }
    }

    // Drop the employee table, in case it exists
    $drop = 'DROP TABLE EMPLOYEE';
    $res = @db2_exec($conn, $drop);

    // Create the employee table
    $create = 'CREATE TABLE EMPLOYEE (EMPNO CHAR(6) NOT NULL, FIRSTNME VARCHAR(12) NOT NULL, MIDINIT CHAR(1) NOT NULL, LASTNAME VARCHAR(15) NOT NULL, WORKDEPT CHAR(3), PHONENO CHAR(4), HIREDATE DATE, JOB CHAR(8), EDLEVEL SMALLINT NOT NULL, SEX CHAR(1), BIRTHDATE DATE, SALARY DECIMAL (9,2), BONUS DECIMAL(9,2), COMM DECIMAL(9,2))';
    $res = db2_exec($conn, $create);

    // Populate the employee table
    $employee = array(
      array('000010', 'CHRISTINE', 'I', 'HAAS',       'A00', '3978', '1965-01-01', 'PRES',     18, 'F', '1933-08-24', 52750.00, 1000, 4220),
      array('000020', 'MICHAEL',   'L', 'THOMPSON',   'B01', '3476', '1973-10-10', 'MANAGER',  18, 'M' ,'1948-02-02', 41250.00,  800, 3300),
      array('000030', 'SALLY',     'A', 'KWAN',       'C01', '4738', '1975-04-05', 'MANAGER',  20, 'F' ,'1941-05-11', 38250.00,  800, 3060),
      array('000050', 'JOHN',      'B', 'GEYER',      'E01', '6789', '1949-08-17', 'MANAGER',  16, 'M' ,'1925-09-15', 40175.00,  800, 3214),
      array('000060', 'IRVING',    'F', 'STERN',      'D11', '6423', '1973-09-14', 'MANAGER',  16, 'M' ,'1945-07-07', 32250.00,  500, 2580),
      array('000070', 'EVA',       'D', 'PULASKI',    'D21', '7831', '1980-09-30', 'MANAGER',  16, 'F' ,'1953-05-26', 36170.00,  700, 2893),
      array('000090', 'EILEEN',    'W', 'HENDERSON',  'E11', '5498', '1970-08-15', 'MANAGER',  16, 'F' ,'1941-05-15', 29750.00,  600, 2380),
      array('000100', 'THEODORE',  'Q', 'SPENSER',    'E21', '0972', '1980-06-19', 'MANAGER',  14, 'M' ,'1956-12-18', 26150.00,  500, 2092),
      array('000110', 'VINCENZO',  'G', 'LUCCHESSI',  'A00', '3490', '1958-05-16', 'SALESREP', 19, 'M' ,'1929-11-05', 46500.00,  900, 3720),
      array('000120', 'SEAN',      '' , "O'CONNELL",   'A00', '2167', '1963-12-05', 'CLERK',    14, 'M' ,'1942-10-18', 29250.00,  600, 2340),
      array('000130', 'DOLORES',   'M', 'QUINTANA',   'C01', '4578', '1971-07-28', 'ANALYST',  16, 'F' ,'1925-09-15', 23800.00,  500, 1904),
      array('000140', 'HEATHER',   'A', 'NICHOLLS',   'C01', '1793', '1976-12-15', 'ANALYST',  18, 'F' ,'1946-01-19', 28420.00,  600, 2274),
      array('000150', 'BRUCE',     '' , 'ADAMSON',    'D11', '4510', '1972-02-12', 'DESIGNER', 16, 'M' ,'1947-05-17', 25280.00,  500, 2022),
      array('000160', 'ELIZABETH', 'R', 'PIANKA',     'D11', '3782', '1977-10-11', 'DESIGNER', 17, 'F' ,'1955-04-12', 22250.00,  400, 1780),
      array('000170', 'MASATOSHI', 'J', 'YOSHIMURA',  'D11', '2890', '1978-09-15', 'DESIGNER', 16, 'M' ,'1951-01-05', 24680.00,  500, 1974),
      array('000180', 'MARILYN',   'S', 'SCOUTTEN',   'D11', '1682', '1973-07-07', 'DESIGNER', 17, 'F' ,'1949-02-21', 21340.00,  500, 1707),
      array('000190', 'JAMES',     'H', 'WALKER',     'D11', '2986', '1974-07-26', 'DESIGNER', 16, 'M' ,'1952-06-25', 20450.00,  400, 1636),
      array('000200', 'DAVID',     '' , 'BROWN',      'D11', '4501', '1966-03-03', 'DESIGNER', 16, 'M' ,'1941-05-29', 27740.00,  600, 2217),
      array('000210', 'WILLIAM',   'T', 'JONES',      'D11', '0942', '1979-04-11', 'DESIGNER', 17, 'M' ,'1953-02-23', 18270.00,  400, 1462),
      array('000220', 'JENNIFER',  'K', 'LUTZ',       'D11', '0672', '1968-08-29', 'DESIGNER', 18, 'F' ,'1948-03-19', 29840.00,  600, 2387),
      array('000230', 'JAMES',     'J', 'JEFFERSON',  'D21', '2094', '1966-11-21', 'CLERK',    14, 'M' ,'1935-05-30', 22180.00,  400, 1774),
      array('000240', 'SALVATORE', 'M', 'MARINO',     'D21', '3780', '1979-12-05', 'CLERK',    17, 'M' ,'1954-03-31', 28760.00,  600, 2301),
      array('000250', 'DANIEL',    'S', 'SMITH',      'D21', '0961', '1969-10-30', 'CLERK',    15, 'M' ,'1939-11-12', 19180.00,  400, 1534),
      array('000260', 'SYBIL',     'P', 'JOHNSON',    'D21', '8953', '1975-09-11', 'CLERK',    16, 'F' ,'1936-10-05', 17250.00,  300, 1380),
      array('000270', 'MARIA',     'L', 'PEREZ',      'D21', '9001', '1980-09-30', 'CLERK',    15, 'F' ,'1953-05-26', 27380.00,  500, 2190),
      array('000280', 'ETHEL',     'R', 'SCHNEIDER',  'E11', '8997', '1967-03-24', 'OPERATOR', 17, 'F' ,'1936-03-28', 26250.00,  500, 2100),
      array('000290', 'JOHN',      'R', 'PARKER',     'E11', '4502', '1980-05-30', 'OPERATOR', 12, 'M' ,'1946-07-09', 15340.00,  300, 1227),
      array('000300', 'PHILIP',    'X', 'SMITH',      'E11', '2095', '1972-06-19', 'OPERATOR', 14, 'M' ,'1936-10-27', 17750.00,  400, 1420),
      array('000310', 'MAUDE',     'F', 'SETRIGHT',   'E11', '3332', '1964-09-12', 'OPERATOR', 12, 'F' ,'1931-04-21', 15900.00,  300, 1272),
      array('000320', 'RAMLAL',    'V', 'MEHTA',      'E21', '9990', '1965-07-07', 'FIELDREP', 16, 'M' ,'1932-08-11', 19950.00,  400, 1596),
      array('000330', 'WING',      '' , 'LEE',        'E21', '2103', '1976-02-23', 'FIELDREP', 14, 'M' ,'1941-07-18', 25370.00,  500, 2030),
      array('000340', 'JASON',     'R', 'GOUNOT',     'E21', '5698', '1947-05-05', 'FIELDREP', 16, 'M' ,'1926-05-17', 23840.00,  500, 1907)
    );

    $insert = "INSERT INTO EMPLOYEE(EMPNO, FIRSTNME, MIDINIT, LASTNAME, WORKDEPT, PHONENO, HIREDATE, JOB, EDLEVEL, SEX, BIRTHDATE, SALARY, BONUS, COMM) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach ($employee as $row){
           $res = db2_execute($stmt, $row);
        }
    }

    // Drop the emp_photo table, in case it exists
    $drop = 'DROP TABLE EMP_PHOTO';
    $res = @db2_exec($conn, $drop);

    // Create the emp_photo table
    $create = 'CREATE TABLE EMP_PHOTO (EMPNO CHAR(6) NOT NULL, PHOTO_FORMAT VARCHAR(10) NOT NULL, PICTURE BLOB(100K), PRIMARY KEY(EMPNO, PHOTO_FORMAT))';
    $res = db2_exec($conn, $create);

    // Populate the emp_photo table
    $emp_photo = array(
      array('000130', 'jpg', 'pic1.jpg'),
      array('000130', 'png', 'spook.png'),
      array('000140', 'jpg', 'pic1.jpg'),
      array('000140', 'png', 'spook.png'),
      array('000150', 'jpg', 'pic1.jpg'),
      array('000150', 'png', 'spook.png'),
      array('000190', 'jpg', 'pic1.jpg'),
      array('000190', 'png', 'spook.png')
    );

    $insert = 'INSERT INTO EMP_PHOTO (EMPNO, PHOTO_FORMAT, PICTURE) VALUES (?, ?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach ($emp_photo as $row){
           $empno = $row[0];
           $photo_format = $row[1];
           $picname = $row[2];
           $picture = dirname(__FILE__) . "/$picname";
           db2_bind_param($stmt, 1, "empno", DB2_PARAM_IN);
           db2_bind_param($stmt, 2, "photo_format", DB2_PARAM_IN);
           $res = db2_bind_param($stmt, 3, "picture", DB2_PARAM_FILE, DB2_BINARY);
           $res = db2_execute($stmt);
        }
    }

    // Drop the table org, in case it exists
    $drop = 'DROP TABLE ORG';
    $res = @db2_exec($conn, $drop);

    // Create the table org
    $create = 'CREATE TABLE ORG (DEPTNUMB SMALLINT NOT NULL, DEPTNAME VARCHAR(14), MANAGER SMALLINT, DIVISION VARCHAR(10), LOCATION VARCHAR(13))';
    $res = db2_exec($conn, $create);

    // Populate the org table
    $org = array(
      array(10, 'Head Office',    160, 'Corporate', 'New York'),
      array(15, 'New England',    50,  'Eastern',   'Boston'),
      array(20, 'Mid Atlantic',   10,  'Eastern',   'Washington'),
      array(38, 'South Atlantic', 30,  'Eastern',   'Atlanta'),
      array(42, 'Great Lakes',    100, 'Midwest',   'Chicago'),
      array(51, 'Plains',         140, 'Midwest',   'Dallas'),
      array(66, 'Pacific',        270, 'Western',   'San Francisco'),
      array(84, 'Mountain',       290, 'Western',   'Denver')
    );

    $insert = 'INSERT INTO ORG (DEPTNUMB, DEPTNAME, MANAGER, DIVISION, LOCATION) VALUES (?, ?, ?, ?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
       foreach ($org as $row){
          $res = db2_execute($stmt, $row);
       }
    }

    // Drop the project table, in case it exists
    $drop = 'DROP TABLE PROJECT';
    $res = @db2_exec($conn, $drop);

    // Create the project table
    $create = 'CREATE TABLE PROJECT (PROJNO CHAR(6) NOT NULL, PROJNAME VARCHAR(24) NOT NULL, DEPTNO CHAR(3) NOT NULL, RESPEMP CHAR(6) NOT NULL, PRSTAFF DECIMAL(5,2), PRSTDATE DATE, PRENDATE DATE, MAJPROJ CHAR(6))';
    $res = db2_exec($conn, $create);

    // Populate the project table
    $project = array(
      array('AD3100', 'ADMIN SERVICES',        'D01', '000010', 6.5, '1982-01-01', '1983-02-01', ''),
      array('AD3110', 'GENERAL ADMIN SYSTEMS', 'D21', '000070',   6, '1982-01-01', '1983-02-01', 'AD3100'),
      array('AD3111', 'PAYROLL PROGRAMMING',   'D21', '000230',   2, '1982-01-01', '1983-02-01', 'AD3110'),
      array('AD3112', 'PERSONNEL PROGRAMMING', 'D21', '000250',   1, '1982-01-01', '1983-02-01', 'AD3110'),
      array('AD3113', 'ACCOUNT PROGRAMMING',   'D21', '000270',   2, '1982-01-01', '1983-02-01', 'AD3110'),
      array('IF1000', 'QUERY SERVICES',        'C01', '000030',   2, '1982-01-01', '1983-02-01', null),
      array('IF2000', 'USER EDUCATION',        'C01', '000030',   1, '1982-01-01', '1983-02-01', null),
      array('MA2100', 'WELD LINE AUTOMATION',  'D01', '000010',  12, '1982-01-01', '1983-02-01', null),
      array('MA2110', 'W L PROGRAMMING',       'D11', '000060',   9, '1982-01-01', '1983-02-01', 'MA2100'),
      array('MA2111', 'W L PROGRAM DESIGN',    'D11', '000220',   2, '1982-01-01', '1982-12-01', 'MA2110'),
      array('MA2112', 'W L ROBOT DESIGN',      'D11', '000150',   3, '1982-01-01', '1982-12-01', 'MA2110'),
      array('MA2113', 'W L PROD CONT PROGS',   'D11', '000160',   3, '1982-02-15', '1982-12-01', 'MA2110'),
      array('OP1000', 'OPERATION SUPPORT',     'E01', '000050',   6, '1982-01-01', '1983-02-01', null),
      array('OP1010', 'OPERATION',             'E11', '000090',   5, '1982-01-01', '1983-02-01', 'OP1000'),
      array('OP2000', 'GEN SYSTEMS SERVICES',  'E01', '000050',   5, '1982-01-01', '1983-02-01', null),
      array('OP2010', 'SYSTEMS SUPPORT',       'E21', '000100',   4, '1982-01-01', '1983-02-01', 'OP2000'),
      array('OP2011', 'SCP SYSTEMS SUPPORT',   'E21', '000320',   1, '1982-01-01', '1983-02-01', 'OP2010'),
      array('OP2012', 'APPLICATIONS SUPPORT',  'E21', '000330',   1, '1982-01-01', '1983-02-01', 'OP2010'),
      array('OP2013', 'DB/DC SUPPORT',         'E21', '000340',   1, '1982-01-01', '1983-02-01', 'OP2010'),
      array('PL2100', 'WELD LINE PLANNING',    'B01', '000020',   1, '1982-01-01', '1982-09-15', 'MA2100')
    );

    $insert = 'INSERT INTO PROJECT (PROJNO, PROJNAME, DEPTNO, RESPEMP, PRSTAFF, PRSTDATE, PRENDATE, MAJPROJ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach($project as $row){
           $res = db2_execute($stmt, $row);
        }
    }

    // Drop the sales table, in case it exists
    $drop = 'DROP TABLE SALES';
    $res = @db2_exec($conn, $drop);

    // Create the sales table
    $create = 'CREATE TABLE SALES (SALES_DATE DATE, SALES_PERSON VARCHAR(15), REGION VARCHAR(15), SALES INT)';
    $res = db2_exec($conn, $create);

    // Populate the sales table
    $sales = array(
      array('1995-12-31', 'LUCCHESSI',   'Ontario-South',  1),
      array('1995-12-31', 'LEE',         'Ontario-South',  3),
      array('1995-12-31', 'LEE',         'Quebec',         1),
      array('1995-12-31', 'LEE',         'Manitoba',       2),
      array('1995-12-31', 'GOUNOT',      'Quebec',         1),
      array('1996-03-29', 'LUCCHESSI',   'Ontario-South',  3),
      array('1996-03-29', 'LUCCHESSI',   'Quebec',         1),
      array('1996-03-29', 'LEE',         'Ontario-South',  2),
      array('1996-03-29', 'LEE',         'Ontario-North',  2),
      array('1996-03-29', 'LEE',         'Quebec',         3),
      array('1996-03-29', 'LEE',         'Manitoba',       5),
      array('1996-03-29', 'GOUNOT',      'Ontario-South',  3),
      array('1996-03-29', 'GOUNOT',      'Quebec',         1),
      array('1996-03-29', 'GOUNOT',      'Manitoba',       7),
      array('1996-03-30', 'LUCCHESSI',   'Ontario-South',  1),
      array('1996-03-30', 'LUCCHESSI',   'Quebec',         2),
      array('1996-03-30', 'LUCCHESSI',   'Manitoba',       1),
      array('1996-03-30', 'LEE',         'Ontario-South',  7),
      array('1996-03-30', 'LEE',         'Ontario-North',  3),
      array('1996-03-30', 'LEE',         'Quebec',         7),
      array('1996-03-30', 'LEE',         'Manitoba',       4),
      array('1996-03-30', 'GOUNOT',      'Ontario-South',  2),
      array('1996-03-30', 'GOUNOT',      'Quebec',        18),
      array('1996-03-30', 'GOUNOT',      'Manitoba',       1),
      array('1996-03-31', 'LUCCHESSI',   'Manitoba',       1),
      array('1996-03-31', 'LEE',         'Ontario-South', 14),
      array('1996-03-31', 'LEE',         'Ontario-North',  3),
      array('1996-03-31', 'LEE',         'Quebec',         7),
      array('1996-03-31', 'LEE',         'Manitoba',       3),
      array('1996-03-31', 'GOUNOT',      'Ontario-South',  2),
      array('1996-03-31', 'GOUNOT',      'Quebec',         1),
      array('1996-04-01', 'LUCCHESSI',   'Ontario-South',  3),
      array('1996-04-01', 'LUCCHESSI',   'Manitoba',       1),
      array('1996-04-01', 'LEE',         'Ontario-South',  8),
      array('1996-04-01', 'LEE',         'Ontario-North', null),
      array('1996-04-01', 'LEE',         'Quebec',         8),
      array('1996-04-01', 'LEE',         'Manitoba',       9),
      array('1996-04-01', 'GOUNOT',      'Ontario-South',  3),
      array('1996-04-01', 'GOUNOT',      'Ontario-North',  1),
      array('1996-04-01', 'GOUNOT',      'Quebec',         3),
      array('1996-04-01', 'GOUNOT',      'Manitoba',       7)
    );

    $insert = 'INSERT INTO SALES (SALES_DATE, SALES_PERSON, REGION, SALES) VALUES (?, ?, ?, ?)';
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach($sales as $row){
           $res = db2_execute($stmt, $row);
        }
    }

    // Drop the stored procedure, in case it exists
    $drop = 'DROP PROCEDURE MATCH_ANIMAL';
    $res = @db2_exec($conn, $drop);

    // Create the stored procedure
    $create = 'CREATE PROCEDURE match_animal(IN first_name VARCHAR(128), INOUT second_name VARCHAR(128), OUT animal_weight DOUBLE) DYNAMIC RESULT SETS 1 LANGUAGE SQL BEGIN DECLARE match_name INT DEFAULT 0; DECLARE c1 CURSOR FOR SELECT COUNT(*) FROM animals WHERE name IN (second_name); DECLARE c2 CURSOR FOR SELECT SUM(weight) FROM animals WHERE name in (first_name, second_name); DECLARE c3 CURSOR WITH RETURN FOR SELECT name, breed, weight FROM animals WHERE name BETWEEN first_name AND second_name ORDER BY name; OPEN c1; FETCH c1 INTO match_name; IF (match_name > 0) THEN SET second_name = \'TRUE\'; END IF; CLOSE c1; OPEN c2; FETCH c2 INTO animal_weight; CLOSE c2; OPEN c3; END';
    $res = db2_exec($conn, $create);

    // Drop the staff table, in case it exists
    $drop = 'DROP TABLE STAFF';
    $res = @db2_exec($conn, $drop);

    // Create the staff table
    $create = 'CREATE TABLE STAFF (ID SMALLINT NOT NULL, NAME VARCHAR(9), DEPT SMALLINT, JOB CHAR(5), YEARS SMALLINT, SALARY DECIMAL(7,2), COMM DECIMAL(7,2))';
    $res = db2_exec($conn, $create);

    // Populate the staff table
    $staff = array(
      array(10, 'Sanders',    20, 'Mgr',   7,    18357.50, null),
      array(20, 'Pernal',     20, 'Sales', 8,    18171.25, 612.45),
      array(30, 'Marenghi',   38, 'Mgr',   5,    17506.75, null),
      array(40, "O'Brien",     38, 'Sales', 6,    18006.00, 846.55),
      array(50, 'Hanes',      15, 'Mgr',   10,   20659.80, null),
      array(60, 'Quigley',    38, 'Sales', null,  16808.30, 650.25),
      array(70, 'Rothman',    15, 'Sales', 7,    16502.83, 1152.00),
      array(80, 'James',      20, 'Clerk', null,  13504.60, 128.20),
      array(90, 'Koonitz',    42, 'Sales', 6,    18001.75, 1386.70),
      array(100, 'Plotz',     42, 'Mgr'  , 7,    18352.80, null),
      array(110, 'Ngan',      15, 'Clerk', 5,    12508.20, 206.60),
      array(120, 'Naughton',  38, 'Clerk', null,  12954.75, 180.00),
      array(130, 'Yamaguchi', 42, 'Clerk', 6,    10505.90, 75.60),
      array(140, 'Fraye',     51, 'Mgr'  , 6,    21150.00, null),
      array(150, 'Williams',  51, 'Sales', 6,    19456.50, 637.65),
      array(160, 'Molinare',  10, 'Mgr'  , 7,    22959.20, null),
      array(170, 'Kermisch',  15, 'Clerk', 4,    12258.50, 110.10),
      array(180, 'Abrahams',  38, 'Clerk', 3,    12009.75, 236.50),
      array(190, 'Sneider',   20, 'Clerk', 8,    14252.75, 126.50),
      array(200, 'Scoutten',  42, 'Clerk', null,  11508.60, 84.20),
      array(210, 'Lu',        10, 'Mgr'  , 10,   20010.00, null),
      array(220, 'Smith',     51, 'Sales', 7,    17654.50, 992.80),
      array(230, 'Lundquist', 51, 'Clerk', 3,    13369.80, 189.65),
      array(240, 'Daniels',   10, 'Mgr'  , 5,    19260.25, null),
      array(250, 'Wheeler',   51, 'Clerk', 6,    14460.00, 513.30),
      array(260, 'Jones',     10, 'Mgr'  , 12,   21234.00, null),
      array(270, 'Lea',       66, 'Mgr'  , 9,    18555.50, null),
      array(280, 'Wilson',    66, 'Sales', 9,    18674.50, 811.50),
      array(290, 'Quill',     84, 'Mgr'  , 10,   19818.00, null),
      array(300, 'Davis',     84, 'Sales', 5,    15454.50, 806.10),
      array(310, 'Graham',    66, 'Sales', 13,   21000.00, 200.30),
      array(320, 'Gonzales',  66, 'Sales', 4,    16858.20, 844.00),
      array(330, 'Burke',     66, 'Clerk', 1,    10988.00, 55.50),
      array(340, 'Edwards',   84, 'Sales', 7,    17844.00, 1285.00),
      array(350, 'Gafney',    84, 'Clerk', 5,    13030.50, 188.00)
    );

    $insert = "INSERT INTO STAFF (ID, NAME, DEPT, JOB, YEARS, SALARY, COMM) VALUES (?, ?, ?, ?, ?, ?, ?)";
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach($staff as $row){
           $res = db2_execute($stmt, $row);
        }
    }

    // Drop the emp_resume table, in case it exists
    $drop = "DROP TABLE EMP_RESUME";
    $res = @db2_exec($conn, $drop);

    // Create the emp_resume table
    $create = "CREATE TABLE EMP_RESUME (EMPNO CHAR(6) NOT NULL, RESUME_FORMAT VARCHAR(10) NOT NULL, RESUME CLOB(5K))";
    $res = db2_exec($conn, $create);

    // Populate the emp_resume table
    $resume = array(
      array('000130', 'ascii', 'resume_000130.txt'),
      array('000140', 'ascii', 'resume_000140.txt'),
      array('000150', 'ascii', 'resume_000150.txt'),
      array('000190', 'ascii', 'resume_000190.txt')
    );

    $insert = "INSERT INTO EMP_RESUME (EMPNO, RESUME_FORMAT, RESUME) VALUES (?, ?, ?)";
    $stmt = db2_prepare($conn, $insert);
    if($stmt){
        foreach($resume as $row){
           $empno = $row[0];
           $resume_format = $row[1];
           $resumename = $row[2];
           $resume = dirname(__FILE__) . "/$resumename";
           db2_bind_param($stmt, 1, "empno", DB2_PARAM_IN);
           db2_bind_param($stmt, 2, "resume_format", DB2_PARAM_IN);
           db2_bind_param($stmt, 3, "resume", DB2_PARAM_FILE, DB2_BINARY);
           $res = db2_execute($stmt);
        }
    }

    // Drop the t_string table, in case it exists
    $drop = 'DROP TABLE T_STRING';
    $res = @db2_exec($conn, $drop);

    // Create the t_string table
    $create = 'CREATE TABLE T_STRING (A INTEGER, B DOUBLE, C VARCHAR(100))';
    $res = db2_exec($conn, $create);

    // Drop the test table for PECL bug #6755
    $drop = 'DROP TABLE TABLE_6755';
    $result = @db2_exec($conn, $drop);

    // Drop the test table for PECL bug #6792
    $drop = 'DROP TABLE TABLE_6792';
    $result = @db2_exec($conn, $drop);

    // Drop the test table for XML testing
    $drop = 'DROP TABLE xmlTest';
    $result = @db2_exec($conn, $drop);

    // Drop the test table for primary keys
    $drop = 'DROP TABLE TEST_PRIMARY_KEYS';
    $result = @db2_exec($conn, $drop);

    // Drop the test table for foreign keys
    $drop = 'DROP TABLE TEST_FOREIGN_KEYS';
    $result = @db2_exec($conn, $drop);


}

?>
--EXPECT--
