#!/bin/bash

## Server configuration
REMOTE_IP="52.207.77.238"
REPETITIONS=100

size=$1

netperf -p 10000 -a 4096,4096 -A 4096,4096 -H ${REMOTE_IP} -t TCP_RR -j -l -${REPETITIONS} -- -o min_latency,max_latency,mean_latency,P50_LATENCY,P90_LATENCY,P99_LATENCY,stddev_latency -r${size},${size} -P,10001 > ethernet_${size}
