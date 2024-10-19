

* PraaS repository: `benchmarks` branch.
* Functions source code: `benchmarks/swapper/functions`
* Benchmarks source code:  `benchmarks/swapper/`
* Container build?: `benchmarks` branch, `process/docker/Dockerfile.process-swapper`. First might be needed to build the images with cpp builder and dependencies.
  ```
  docker build -f process/docker/Dockerfile.process-swapper -t spcleth/praas-examples:process-swapper .
  ```
* Original image: `spcleth/praas:process-swapper` and `spcleth/praas-examples:process-swapper`
