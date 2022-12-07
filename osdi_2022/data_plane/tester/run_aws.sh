#!/bin/bash

CUR_DIR=$(pwd)
REPETITIONS=200

URL="https://noyk8gdlol.execute-api.us-east-1.amazonaws.com/default/hello-world"
AUTH=""

#SIZE_STEPS="1 2 4 8 16 32 48 64 80 96 112 125 128 256 512 1024 2048 3072 4096 5192"
SIZE_STEPS="1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 3145728 4194304 5242880"

for size in ${SIZE_STEPS}
do
  echo "Perform 200 repetitions of size ${size}"
  ./tester $URL "" ${size} $REPETITIONS 5 > result_aws_$size.csv
  head result_$size.csv
  tail result_$size.csv
done
