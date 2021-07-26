#!/bin/bash

echo `g++ test_$1.cc thread.o libinterrupt.a -ldl -o test`

