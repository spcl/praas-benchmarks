#!/bin/bash


## Server configuration
# Server hosting rFaaS executor
REMOTE_IP="54.221.51.120"
REMOTE_PORT=10000

LOCAL_PORT=10001
# Select the RDMA device for perftest
DEVICE_FLAGS=" -d mlx5_1"

## Benchmark configuration
REPETITIONS=200
RX_DEPTH=32
WARMUP=100
CUR_DIR=$(pwd)

SIZE_STEPS="1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 3145728 4194304 5242880"

mkdir -p results_ethernet
mkdir -p results_rdma

#ssh -f -M $REMOTE "killall netserver"
#ssh -f -M $REMOTE "killall executor"
#ssh -f -M $REMOTE "killall ib_write_lat"
for size in ${SIZE_STEPS}
do

  echo "Execute ethernet, $size"
  #ssh -f -M $REMOTE "taskset 0x3 $NETPERF/netserver"
  #sleep 0.5
  netperf -a 4096,4096 -A 4096,4096 -H ${REMOTE_IP} -t TCP_RR -j -l -${REPETITIONS} -- -o min_latency,max_latency,mean_latency,P50_LATENCY,P90_LATENCY,P99_LATENCY,stddev_latency -r${size},${size} > results_ethernet/ethernet_client_${size}
  #ssh -M $REMOTE "killall netserver"

  #sleep 1
done

