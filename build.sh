#!/bin/bash

make clean
make all

# On Unix-like OSes
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
	export LD_LIBRARY_PATH=lib
elif [[ "$OSTYPE" == "darwin"* ]]; then
        # Mac OSX
	export DYLD_LIBRARY_PATH=lib
fi
