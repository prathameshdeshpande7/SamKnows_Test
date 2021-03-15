Solution for SamKnows Test

https://github.com/SamKnows/tests-and-metrics-test

This is the main README file for this project.

Structure of the project
------------------------
	.
	├── app
	│   ├── Makefile
	│   └── mycurl_test.c
	├── build.sh
	├── lib
	│   ├── Makefile
	│   ├── mycurl.c
	│   └── mycurl.h
	├── Makefile
	├── README.md
	├── run.sh
	└── test_output
	    ├── sample_output.txt
	    ├── sample_response_body.txt
	    ├── syslog_linux.txt
	    └── valgrind_test.txt

	3 directories, 13 files

- The top-level directory contains lib/, app/ and test_output/ folders.

- lib/ contains the curl library code which executes the HTTP requests.

- app/ contains the test program that uses the library in lib/

- test_output/ contains the sample output from one of the runs, there is
also a sample_response_body.txt along with a valgrind memory leak check
report on the entire application and a sample syslog captured on linux.

- The main Makefile invokes the Makefile's within lib/ and app/

- build.sh provides support for make clean, make all and setting the
LD_LIBRARY_PATH for Linux and DYLD_LIBRARY_PATH for macOS.

- run.sh provides a test script to run the application.

How to build and install
------------------------

On Linux

- make clean

- make all

- export LD_LIBRARY_PATH=lib

On MacOSX

- make clean

- make all

- export DYLD_LIBRARY_PATH=lib

Use the above commands to clean and compile the test program.
Alternatively, execute 'source ./build.sh' on the command line which will
clean, make and configure the shared library path correctly.

How to run
----------

- The most basic request is as follows:
	./app/mycurl_test  -H "Content-Type: application/json" -n 10 --header "Accept: application/xml"  -X GET --url http://www.google.com/

- The request type '-X' and '--url' are mandatory arguments.

- Number of requests '-n' is an optional argument, default n is set to a
single request. If '-n' is passed a negative value, n is reset to 1.

On Linux

- Use run.sh script to configure the number of requests, URL to send HTTP
request to, pass headers to the request and set the request type.
Then run ./run.sh to start the tests. See the sample_output.txt file
for reference.

On MacOSX

- Run the command inside run.sh manually to test the application.

Features
--------

- Supported on

	CentOS Linux release 8.2.2004 (Core) kernel version 4.18.0-147.8.1.el8_1.x86_64

	macOS Catalina version 10.15.5

- Currently, only 'GET' HTTP request type is supported.

- Multiple HTTP GET requests are executed on a sequential basis, i.e. one
after another as curl_easy_perform performs the entire request in a
blocking manner and returns when done, or if it failed.

- If you intend to stop the run.sh script in the middle of executing a large
number of requests (for example more than 1000 or so), press CTRL^C twice
in quick succession for stopping the script.

- Sample output of 10 sequential GET requests executed for 10 iterations can
be seen in sample_output.txt file.

- All the INFO logs are logged to syslog as well as on the console. The main
advantage of logging into syslog or /var/log/messages is that if the host
system crashes or shuts down due to power loss or for any other unknown
reason, we ensure that the last executed request is logged in the system.
This is easier to rootcause requests which are persistently failing or
being remotely debugged on systems who may not not have console access.

- The headers and the response body are not dumped either on the console or
in syslog because of the size of the data. However, it can be enabled in
the future versions if needed. Alternatively, sample response body can be
seen in sample_response_body.txt

Test setup and tests run
------------------------

- Linux setup was directly connected to the internet using a physical Ethernet
LAN connection.

- macOS was connected using WiFi to the Internet service provider.

- Testing was done by issuing requests while connected to the network.

- Negative testing was done by abruptly closing the internet connection either
by removing the LAN cable, disconnecting WiFi or powering off the modem and
router.

- The app will abort requests if the mandatory fields are missing on the
command-line.

TODO
----

- Porting on Windows is remaining, currently working on a forked origin/windows
branch for adding this support. A more generic method to parse command-line
parameters is required on Windows instead of getopt_long which works well
for Linux and macOS.

- For non-blocking behavior, curl_multi_perform is not yet supported in this
version. This will enable us to issue simultaneous requests in parallel and
speed-up processing.

- Support for other HTTP request types such as PUT, POST and DELETE yet to
be added.

- Support for accepting username and password fields while issuing a request
yet to be added.

- Support for a flag such as --dump-headers to a file can be added.

- The application requires a full fledged test-suite covering all possible
success, failure and corner cases.

- Wireshark TCP packet capture needs to be used to check the packets actually
arriving on the network and over the TCP connection.

- Performance of the network requests yet to be analysed.

- Bubble sort is used to compute median at the moment so the complexity is O(N^2)
where N is the number of requests. Time complexity for sorting can be reduced by
using merge sort O(NlogN) best-case instead of bubble sort.

Known Issues
------------

On MacOSX

- Currently, run.sh is not working as expected and throwing error such as:

dyld: Library not loaded: libmycurl.dylib
Referenced from: /path/SamKnows_Test/./app/mycurl_test
Reason: image not found

Manually executing the application works as expected.

- Earlier version computed mean of the values instead of median. This version
calculates it correctly.
