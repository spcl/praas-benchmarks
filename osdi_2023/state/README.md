

* PraaS repository: `benchmarks` branch.
* Functions source code: `benchmarks/state/functions`
* Benchmarks source code:  `benchmarks/state/`
* Container build?: `benchmarks` branch, `process/docker/Dockerfile.process-state`. First might be needed to build the images with cpp builder and dependencies.
  ```
  docker build -f process/docker/Dockerfile.process-state -t spcleth/praas-examples:process-state .
  ```
* Original image: `spcleth/praas:process-state` and `spcleth/praas-examples:process-state`
