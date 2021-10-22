# IBM_DB2

Interface for PHP to DB2 for z/OS, DB2 for LUW, DB2 for i.

## IBM i users

When running on IBM i, `IBM_DB2` doesn't link with the Db2 LUW client library,
but instead with libdb400, which provides a PASE wrapper for SQL/CLI. The
differences between SQL/CLI in IBM i and the LUW driver are wrapped for you.
You don't need Db2 Connect on IBM i as a result.

To install, make sure you have the new Yum-based OSS environment. Install PHP,
plus any dependencies like so:

```shell
yum install sqlcli-devel gcc make-gnu
```

Tony Cairns' [replacement libdb400](https://bitbucket.org/litmis/db2sock/src/master/db2/)
is not yet tested, but may be desirable due to its greater debugging features.

## LUW/z/Db2 Connect users

CLIDRIVER should be installed in your system.
If not installed Download from the below link.

<a name="downloadCli"></a> [DOWNLOAD CLI DRIVER](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/)

PHP, gcc, make, tar should be installed in your system.

You may not find gcc, make, tar in some of the docker containers (Example Amazon Linux2).
In such cases use below command to install gcc etc.
```shell
yum install make gcc
```
## How to install php ibm_db2 extension in Linux/Mac.
```
if IBM_DB_HOME and LD_LIBRARY_PATH environment variable not set then set them with installed CLIDRIVER.
(say CLIDRIVER installed at "/home/user/clidriver")

export IBM_DB_HOME=/home/user/clidriver 
export LD_LIBRARY_PATH=/home/user/clidriver/lib
export PATH=/home/user/clidriver/bin:$PATH

In case of Docker(Example Amazon Linux2):
  execute 'db2level' command in your command prompt, If any error comes then install 'pam' from package manager.
  yum install pam

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
## Prebuilt binaries for Windows

1. Add the `CLIDRIVER\bin` path to the `PATH` environment variable like so (for a batch file):
    ```
    set PATH=<CLIDRIVER installed path>\bin;%PATH%
    ```
2. Download the DLLs for PHP 7.x and 8.x from [the ibmdb repository](https://github.com/ibmdb/php_ibm_db2).
   Select the build for the PHP that matches the version, architecture, and thread model.

3. Open the `php.ini` file in an editor of your choice. Edit the extension entry in the
   `php.ini` file in the `<local_php_directory>\php\lib` directory to reference the driver:
    ````
    extension=php_ibm_db2
    ````

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

## Test suite

`make test` is adequate for most tests except those involving XMLSERVICE.
For more complex tests, use `run-tests.php` and set `TEST_PHP_ARGS` as needed.

Many tests rely on having libraries like `DB2` added. On IBM i, `create schema`
instead of `crtlib` is recommended to properly set up things like journals.
Please let us know if there are any hardcoded library names; we've tried to
clean up usage of them when not documented.

If you have strange failures during the tests, you might want to adjust the
autocommit setting. `ibm_db2.i5_allow_commit=1` is tested to work.

On IBM i, you might want to use `*LOCAL` as your DSN without a username or
password. This means you won't need to provide another account for most tests.
Modify `tests/connection.inc` to do so.

On IBM i, it's strongly recommended to set QCCSID to a reasonable value, not
65535. Without setting this, you will have many string values failing to
convert from EBCDIC. `IBM_DB2` provides a workaround for this nowadays due to
the amount of systems in the wild without a properly set QCCSID, but you
should do this anyways. To check and set QCCSID, run `WRKSYSVAL` from a 5250.

## Contributing:
```
See CONTRIBUTING.md

The developer sign-off should include the reference to the DCO in defect remarks(example below):
DCO 1.1 Signed-off-by: Random J Developer <random@developer.org>
```
