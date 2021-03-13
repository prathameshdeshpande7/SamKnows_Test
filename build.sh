#!/bin/bash

make clean
make all

# On Unix-like OSes
export LD_LIBRARY_PATH=lib
