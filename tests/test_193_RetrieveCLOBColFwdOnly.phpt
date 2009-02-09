--TEST--
IBM-DB2: retrieve CLOB columns: forward-only cursor
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($database, $user, $password);

$sql = "SELECT empno, resume_format, resume
FROM emp_resume
WHERE resume_format = 'ascii'";

if (!$conn) {
   die('no connection: ' . db2_conn_errormsg());	
}
$result = db2_exec($conn, $sql);	
while ($row = db2_fetch_array($result)) {
  print_r($row);
}
?>
--EXPECTF--
Array
(
    [0] => 000130
    [1] => ascii
    [2] => 

  Resume:  Delores M. Quintana
 
 
  Personal Information
 
  Address:            1150 Eglinton Ave
                      Mellonville, Idaho 83725
  Phone:              (208) 875-9933
  Birthdate:          September 15, 1925
  Sex:                Female
  Marital Status:     Married
  Height:             5'2"
  Weight:             120 lbs.
 
 
  Department Information
 
  Employee Number:    000130
  Dept Number:        C01
  Manager:            Sally Kwan
  Position:           Analyst
  Phone:              (208) 385-4578
  Hire Date:          1971-07-28
 
 
  Education
 
  1965                Math and English, B.A.
                      Adelphi University
 
  1960                Dental Technician
                      Florida Institute of Technology
 
 
  Work History
 
  10/91 - present     Advisory Systems Analyst
                      Producing documentation tools for engineering
                      department.
 
  12/85 - 9/91        Technical Writer
                      Writer, text programmer, and planner.
 
  1/79 - 11/85        COBOL Payroll Programmer
                      Writing payroll programs for a diesel fuel
                      company.
 
 
  Interests
 
  o   Cooking
  o   Reading
  o   Sewing
  o   Remodeling

)
Array
(
    [0] => 000140
    [1] => ascii
    [2] => 

  Resume:  Heather A. Nicholls
 
 
  Personal Information
 
  Address:            844 Don Mills Ave
                      Mellonville, Idaho 83734
  Phone:              (208) 610-2310
  Birthdate:          January 19, 1946
  Sex:                Female
  Marital Status:     Single
  Height:             5'8"
  Weight:             130 lbs.
 
 
  Department Information
 
  Employee Number:    000140
  Dept Number:        C01
  Manager:            Sally Kwan
  Position:           Analyst
  Phone:              (208) 385-1793
  Hire Date:          1976-12-15
 
 
  Education
 
  1972                Computer Engineering, Ph.D.
                      University of Washington
 
  1969                Music and Physics, B.A.
                      Vassar College
 
 
  Work History
 
  2/83 - present      Architect, OCR Development
                      Designing the architecture of OCR products.
 
  12/76 - 1/83        Text Programmer
                      Optical character recognition (OCR) programming in
                      PL/I.
 
  9/72 - 11/76        Punch Card Quality Analyst
                      Checking punch cards met quality specifications.
 
 
  Interests
 
  o   Model railroading
  o   Interior decorating
  o   Embroidery
  o   Knitting

)
Array
(
    [0] => 000150
    [1] => ascii
    [2] => 

  Resume:  Bruce Adamson
 
 
  Personal Information
 
  Address:            3600 Steeles Ave
                      Mellonville, Idaho 83757
  Phone:              (208) 725-4489
  Birthdate:          May 17, 1947
  Sex:                Male
  Marital Status:     Married
  Height:             6'0"
  Weight:             175 lbs.
 
 
  Department Information
 
  Employee Number:    000150
  Dept Number:        D11
  Manager:            Irving Stern
  Position:           Designer
  Phone:              (208) 385-4510
  Hire Date:          1972-02-12
 
 
  Education
 
  1971                Environmental Engineering, M.Sc.
                      Johns Hopkins University
 
  1968                American History, B.A.
                      Northwestern University
 
 
  Work History
 
  8/79 - present      Neural Network Design
                      Developing neural networks for machine
                      intelligence products.
 
  2/72 - 7/79         Robot Vision Development
                      Developing rule-based systems to emulate sight.
 
  9/71 - 1/72         Numerical Integration Specialist
                      Helping bank systems communicate with each other.
 
 
  Interests
 
  o   Racing motorcycles
  o   Building loudspeakers
  o   Assembling personal computers
  o   Sketching

)
Array
(
    [0] => 000190
    [1] => ascii
    [2] => 

  Resume:  James H. Walker


  Personal Information

  Address:            3500 Steeles Ave
                      Mellonville, Idaho 83757
  Phone:              (208) 725-7325
  Birthdate:          June 25, 1952
  Sex:                Male
  Marital Status:     Single
  Height:             5'11"
  Weight:             166 lbs.


  Department Information

  Employee Number:    000190
  Dept Number:        D11
  Manager:            Irving Stern
  Position:           Designer
  Phone:              (208) 385-2986
  Hire Date:          1974-07-26


  Education

  1974                Computer Studies, B.Sc.
                      University of Massachusetts

  1972                Linguistic Anthropology, B.A.
                      University of Toronto


  Work History

  6/87 - present      Microcode Design
                      Optimizing algorithms for mathematical functions.

  4/77 - 5/87         Printer Technical Support
                      Installing and supporting laser printers.

  9/74 - 3/77         Maintenance Programming
                      Patching assembly language compiler for
                      mainframes.


  Interests

  o   Wine tasting
  o   Skiing
  o   Swimming
  o   Dancing

)
