# IBM_DB2

Interface for PHP to DB2 for z/OS, DB2 for LUW, DB2 for i.

## Prerequisite

CLIDRIVER should be installed in your system.
If not installed Download from the below link.

<a name="downloadCli"></a> [DOWNLOAD CLI DRIVER](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/)

PHP should be installed in your system.

## How to install php ibm_db2 extension in Linux/Mac
```
if IBM_DB_HOME and LD_LIBRARY_PATH environment variable not set then set them with installed CLIDRIVER.
(say CLIDRIVER installed at "/home/user/clidriver")

export IBM_DB_HOME=/home/user/clidriver 
export LD_LIBRARY_PATH=/home/user/clidriver/lib

1) pecl install ibm_db2
        
2) Open the php.ini file in an editor of your choice. Edit the extension entry in the
   php.ini file in the <local_php_directory>/php/lib directory to reference the PHP driver:
       extension=ibm_db2.so
       
3) Ensure that the PHP driver can access the libdb2.so CLI driver file by
   setting the LD_LIBRARY_PATH variable for Linux and UNIX operating systems
   other than the AIX® operating system. For AIX operating system, you must set LIBPATH variable. 

4) Optional: If the PHP application that is connecting to an IBM database server is running ini
   the HTTP server environment, add the LD_LIBRARY_PATH variable in the httpd.conf file.

```
## How to install php ibm_db2 extension in Windows
```
Set CLIDRIVER\bin path to PATH environment variable.

set PATH=<CLIDRIVER installed path>\bin;%PATH%

1.  Download the php_ibm_db2 DLLs for PHP 7.x(7.0, 7.1, 7.2) from below link.
      https://github.com/ibmdb/php_ibm_db2
	  
    For PHP 5.x, Download the DLL from Pecl. Pasted the link below.
      https://pecl.php.net/package/ibm_db2
   
2. Open the php.ini file in an editor of your choice. Edit the extension entry in the
   php.ini file in the <local_php_directory>\php\lib directory to reference the PHP driver:
	  extension=php_ibm_db2
```

## How to run sample program

### connect.php:-

```
<?php
$database = 'dsn name';
$user = 'user';
$password = 'password';
$conn = db2_connect($database, $user, $password);

if ($conn) {
    echo "Connection succeeded.";
    db2_close($conn);
}
else {
    echo "Connection failed: " . db2_conn_errormsg();
}
?>

To run the sample:- php connect.php
```
## How to build from source code in Linux or Mac
```
Use the commands included in the source code:
download Source code from https://pecl.php.net/package/ibm_db2
 a) Extract the source archive
 b) Run the following commands from the extracted directory:
      $ phpize --clean
      $ phpize
      $ ./configure --with-IBM_DB2=/home/user/clidriver
      $ make
      $ make install
```
## How to build from source code in Windows
```
Below blog mentiones how to build php ibm_db2 from source in windows.
https://www.ibm.com/developerworks/community/blogs/96960515-2ea1-4391-8170-b0515d08e4da/entry/Install_PHP_ibm_db2_Driver?lang=en

```
## Contributing:
```
See CONTRIBUTING.md

The developer sign-off should include the reference to the DCO in defect remarks(example below):
DCO 1.1 Signed-off-by: Random J Developer <random@developer.org>
```
