#!/bin/bash

NUM=10
H1='Content-Type: application/json'
H2='Accept: application/json'
H3='Accept-Charset: utf-8'
URL='http://www.google.com'
TYPE='GET'

echo "Start of test"

for i in `seq 1 10`;
do
	echo ----------------------------------------------------------------------------------
	echo ITERATION $i
	TEST="$(./app/mycurl_test -X "$TYPE" -H "${H1}" -H "${H2}" -H "${H3}" -n $NUM --url "$URL")"
	echo ----------------------------------------------------------------------------------
	echo
done

echo "All done"
