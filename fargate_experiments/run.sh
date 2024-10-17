#!/bin/bash

for size in 10 1024 102400 1048576 5242880; do
  ./benchmarker_mq server ${size} 100 &
  sleep 1
  ./benchmarker_mq client ${size} 100 result_mq_${size}.csv
done

for size in 10 1024 102400 1048576 5242880; do
  ./benchmarker_uds server ${size} 100 &
  sleep 1
  ./benchmarker_uds client ${size} 100 result_uds_${size}.csv
done
