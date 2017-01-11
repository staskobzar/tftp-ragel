### tftp client

TFTP command line client for getting and putting files to tftp server.
This project is not trying to replace existing tftp clients.
I started it just to learn how to write robust protocol implementation 
using ragel state machine.

It is a playground project and probably is not supposed to be use in production. Also it does not implement RFC1350 extension.

Client is written with C with Apache Portable Runtime. Using cmocka for unit tests and DejaGNU for behaviour testing.

More description is in blog article: 
https://staskobzar.blogspot.ca/2017/01/using-ragel-to-implement-tftp-protocol.html

Requirements:
* ragel
* Apache Portable Runtime
* cmocka and DejaGNU for tests

To build:
```
./configure
make
```

To test:
```
./configure
make
make check
```

Run: ```./src/tftpclient```

```
Usage: tftpclient [OPTION] HOST REMOTE_FILE [LOCAL_FILE]
Get file from TFTP server or put file to TFTP server.
HOST        - Hostname or IP address of TFTP server.
REMOTE_FILE - Source file. When  getting  file, then  it is  remote  file name.
              When sending file to  remote server, this is name of  local file.
LOCAL_FILE  - Destination file. When  getting file, then it is  local file name
              or path where to copy  file from  TFTP server. When  sending file
              to remote server, this is name of file to store on remote server.

Mandatory arguments to long options are mandatory for short options too.

Options:
  -h, --help 
        Print usage and breif help message.
  -P, --port [VALUE]
        Server port. Default: 69.
  -p, --put 
        Put file to remote tftp server.
  -g, --get 
        Get file from remote tftp server.
  -m, --mode [VALUE]
        Transfer mode. Value: octet or ascii. If not set, then default is 'ascii'.
  -v, --verbose 
        Print additional infomation during transfer.
  -d, --debug 
        Print lots of debug data.
  -V, --version 
        Print version.

```
