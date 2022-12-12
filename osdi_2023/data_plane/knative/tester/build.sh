#!/bin/bash

CXX=c++
$CXX tester.cpp cpp-base64/base64.cpp -Icpp-base64 -o tester -lcurl


