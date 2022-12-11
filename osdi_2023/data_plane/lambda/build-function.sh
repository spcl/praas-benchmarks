#!/bin/bash

mkdir function_build
cd function_build
cmake ../function -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=../lambda_sdk_install
make VERBOSE=1
make aws-lambda-package-function

