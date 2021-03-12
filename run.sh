#!/bin/bash

make clean
make all
sudo make install
./mycurl_test -H "Accept: application/json" -H "Content-Type: application/json" -X GET -n 11 --header "Accept: application/xml" --url http://www.google.com/
