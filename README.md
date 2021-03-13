Solution for SamKnows Test

https://github.com/SamKnows/tests-and-metrics-test

This is the main README file for this project.

How to build and install
------------------------

- make clean

- make all

- sudo make install

Use the above commands to clean, compile and install the test program.
Alternatively, issue ./build.sh script which will do the above things.

How to run
----------

- Use run.sh script to configure the number of requests, URL to send HTTP
request to, pass headers to the request and set the request type.
Then run ./run.sh to start of the tests. See the sample_output.txt file
for reference.

Features
--------

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

TODO
----

- For non-blocking behavior, curl_multi_perform needs to be used. This will
enable us to issue simultaneous requests in parallel and speed-up processing.

- Support for other HTTP request types such as PUT, POST and DELETE needs to
be added.

- Support for accepting username and password fields while issuing a request
needs to be added.

- Support for a flag such as --dump-headers to a file can be added.

- The application requires a full fledged test-suite covering all possible
success, failure and corner cases.
