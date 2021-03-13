#!/bin/bash

./mycurl_test -H "Accept: application/json" -H "Content-Type: application/json" -X GET -n 500 --header "Accept: application/xml" --url http://www.google.com/
