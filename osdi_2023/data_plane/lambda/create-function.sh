#!/bin/bash

#aws iam create-role \
#  --role-name lambda-cpp-noop \
#  --assume-role-policy-document file://config/role.json

# copy ARN from role
#aws lambda create-function \
#  --function-name cpp-noop \
#  --role arn:aws:iam::261490803749:role/lambda-cpp-noop\
#  --runtime provided \
#  --timeout 1 \
#  --memory-size 1792 \
#  --handler my_handler \
#  --zip-file fileb://function_build/function.zip

aws lambda create-function-url-config \
  --function-name cpp-noop \
  --auth-type NONE
