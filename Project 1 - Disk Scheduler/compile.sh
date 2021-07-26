#!/bin/bash

echo `g++ disk.cc thread.o libinterrupt.a -ldl -o disk`
