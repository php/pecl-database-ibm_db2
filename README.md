# IBM_DB2

Interface for PHP to DB2 for z/OS, DB2 for LUW. [Db2 for IBM i support is deprecated](#new-implementations).

## Pre-requisites

The minimum PHP version supported by driver is PHP 7.3 and the latest version supported is PHP 8.2.

## LUW/z/Db2 Connect users

[CLI driver](https://www.ibm.com/support/pages/db2-odbc-cli-driver-download-and-installation-information) MUST be installed
in your system. You can find and download the latest versions from [here](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/).

`php`, `gcc`, `make` and `tar` should be installed in your system.

You may not find `gcc`, `make`, `tar` in some of the docker images (like Amazon Linux2).
In such cases use below command to install `gcc`, etc.

```shell
yum install make gcc
```

## Db2 for IBM i

### New implementations

**NOTICE**: IBM [recommends](https://www.ibm.com/support/pages/node/883624) that if you are working on a new [Db2 for IBM i](https://www.ibm.com/support/pages/db2-ibm-i)
connection, you MUST use the [PDO_ODBC](https://www.php.net/manual/en/ref.pdo-odbc.php) or the [ODBC](https://www.php.net/manual/en/book.uodbc.php)
extensions instead with the [ODBC Driver for IBM i Access Client Solutions](https://www.ibm.com/support/pages/odbc-driver-ibm-i-access-client-solutions).

### Existing implementations

If you are using this extension with an existing connection running on IBM i, `IBM_DB2` doesn't link with the Db2 LUW client
library, but instead with libdb400, which provides a PASE wrapper for SQL/CLI. The differences between SQL/CLI in IBM i and
the LUW driver are wrapped for you. You don't need Db2 Connect on IBM i as a result.

To install, make sure you have the new Yum-based OSS environment. Install PHP,
plus any dependencies like so:

```shell
yum install sqlcli-devel gcc make-gnu
```

Tony Cairns' [replacement libdb400](https://bitbucket.org/litmis/db2sock/src/master/db2/)
is not yet tested, but may be desirable due to its greater debugging features.

## How to install php ibm_db2 extension in Linux/Mac.

If `IBM_DB_HOME` and `LD_LIBRARY_PATH` environment variable not set then set them with installed CLIDRIVER.
(say CLIDRIVER installed at `/home/user/clidriver`)

```shell
export IBM_DB_HOME=/home/user/clidriver 
export LD_LIBRARY_PATH="${IBM_DB_HOME}/lib"
export PATH="${IBM_DB_HOME}/bin":$PATH
```

In case of Docker (Example Amazon Linux2), execute `db2level` command in your command prompt. If any error comes then install
`pam` from package manager:

```shell
yum install pam
```

1. Install this extension:

   ```shell
   pecl install ibm_db2
   ```
        
2. Open the `php.ini` file in an editor of your choice. Edit the extension entry in the
   `php.ini` file in the `<local_php_directory>/php/lib` directory to reference the PHP driver:

   ```ini
   extension=ibm_db2.so
   ```
       
3. Ensure that the PHP driver can access the `libdb2.so` CLI driver file by
   setting the `LD_LIBRARY_PATH` variable for Linux and UNIX operating systems
   other than the AIX® operating system. For AIX operating system, you must set `LIBPATH` variable. 

4. Optional: If the PHP application that is connecting to an IBM database server is running ini
   the HTTP server environment, add the `LD_LIBRARY_PATH` variable in the `httpd.conf` file.

## Prebuilt binaries for Windows

1. Add the `CLIDRIVER\bin` path to the `PATH` environment variable like so (for a batch file):
    ```
    set PATH=<CLIDRIVER installed path>\bin;%PATH%
    ```
2. Download the DLLs for PHP 7.x and 8.x from [the ibmdb repository](https://github.com/ibmdb/php_ibm_db2).
   Select the build for the PHP that matches the version, architecture, and thread model.

3. Open the `php.ini` file in an editor of your choice. Edit the extension entry in the
   `php.ini` file in the `<local_php_directory>\php\lib` directory to reference the driver:

    ```ini
    extension=php_ibm_db2
    ```

## How to run sample program

Create a `connect.php` script with the following content:

```php
<?php

$database = 'dsn name';
$user = 'user';
$password = 'password';
$conn = db2_connect($database, $user, $password);

if ($conn) {
    echo 'Connection succeeded.';
    db2_close($conn);
} else {
    echo 'Connection failed: ' . db2_conn_errormsg();
}
?>
```

Run the sample program:

```shell
php connect.php
```

## How to build from source code in Linux or Mac

1. Download Source code from https://pecl.php.net/package/ibm_db2
2. Extract the source archive
3. Run the following commands from the extracted directory:

    ```shell
    phpize --clean
    phpize
    ./configure --with-IBM_DB2=/home/user/clidriver
    make
    make install
    ```
## How to build from source code in Windows

[This blog](https://www.ibm.com/developerworks/community/blogs/96960515-2ea1-4391-8170-b0515d08e4da/entry/Install_PHP_ibm_db2_Driver?lang=en)
mentions how to build php ibm_db2 from source in Windows.

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

See [`CONTRIBUTING.md`](CONTRIBUTING.md).
The developer sign-off should include the reference to the DCO in defect remarks, like in this example:

```
DCO 1.1 Signed-off-by: Random J Developer <random@developer.org>
```
