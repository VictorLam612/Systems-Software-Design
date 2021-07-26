#!/bin/bash

echo `g++ thread.cc ./Test_Suite/test_$1.cc libinterrupt.a -ldl -g -o0 -o test`

