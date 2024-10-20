
# Benchmark

We use the PraaS repository, branch `new-benchmarks`, git commit `a33b92d49e2b9131bb13f41e127b617f654565d0`.

## Lambda

The implementation is contained in two Python files and can be found in `praas/examples/latex-editor-python-s3-v1`.
The Docker image is based on `public.ecr.aws/lambda/python:3.10`, and we install texlive there.

## PraaS

The implementation is contained in two Python files and can be found in `praas/examples/latex-editor-python`.

The Docker image is based on `ubuntu:22.04`, and we install texlive there.

## Services

### update-file

This service replaces a given file in the project.

Lambda: we decode the file sent from JSON and write it to S3.

PraaS: the retrieved data is stored in process state.

### get-file

This service returns a given file from the project.

Lambda: we retrieve the object from S3, encode it (if necessary) and return it.

PraaS: the file is retrieved from process state and returned to the user.

### get-pdf

This service retrieves the most recently compiled PDF.

Lambda: we retrieve the pdf from S3 and send it back.

PraaS: we retrieve the file from local disk and 

### compile 

This service runs standard LaTeX compilation.

Lambda: we retrieve list of objects in S3. We compare the timestamp of each object with the files stored in `/tmp`,
and if the object in the cloud is newer, we download it. Then, we run the compilation and upload the resulting
pdf to S3.

PraaS: we retrieve state keys from process state. For state objects newer than the filesystem, we get data from the state
and store it on the filesystem. Then, we run the compilation and return output to the user.

# Benchmarker

Benchmarking invokers can be found in PraaS `benchmarks` directory. For building, we need the `PRAAS_DEPLOYMENTS="aws"` and `PRAAS_COMPONENTS="sdk;benchmarks"`.

Each editing and retrieval task is evaluated separately for four files: `secs/acks.tex`, `secs/multi.tex`, `secs/core.tex`, and `sampleteaser.pdf`.

The compilation benchmarker runs all three scenarios of recompilation, and then it runs the `get-pdf`.
Each compilation task is evaluated by first updating a selected file(s), and then running recompilation of the PDF.
Scenarios:
* (1) Scenario - we upload empty `secs/acks.text` from the input, which effectively does nothing - no recompilation.
* (2) scenario - we upload empty five files, `acmart.cls`, `sampleteaser.pdf`, `secs/core.tex`, `secs/figure.tex`, and `secs/multi.tex`, which causes a LaTeX failure.
* (3) Full recompilation - we upload all files from the input.

## Input

We use the acmart template as basis for all operations.

## Configurations

There are four configurations of deployment size: `small`, `medium`, `large`, and `largest`.
These correspond to different sizes of Lambda functions, and 0.25 - 2 vCPU allocations on Fargate.

## Lambda

Benchmarker running on AWS Lambda is built as part of PraaS benchmarks, and is called `benchmarker_{mode}_s3`.
Each request is sent as JSON. The benchmark uses `libcurl` to make POST requests to AWS Lambda gateway.

(1) Create an S3 bucket, and change the default bucket `praas-benchmarks` in the code of Lambda functions.

(2) Build Docker image provided in `examples/latex-editor-python-s3`. The reference container is `spcleth/praas-examples:latex-editor-python-s3-v1`.

(3) Create four Lambda functions: `praas-latex-get-pdf`, `praas-latex-compile`, `praas-latex-get-file`, and `praas-latex-update-file`. Use the following override for command `compile.get_pdf`, `compile.compile`, `files.get_file`, `files.update_file`.

(4) Run the benchmark: `<build>/benchmarks/latex-service/benchmarker_latex_{mode}_s3 configs/{mode}-lambda{size}.json` where `mode` is `editing` or `compiling`. Run the benchmarker from artifact, as it needs to find the input files.

## PraaS

Benchmarker running on PraaS is built as part of PraaS benchmarks, and is called `benchmarker_{mode}`.
For consistent measurements with Lambda, request payload is also sent as JSON.
Benchmarker is running on a VM, and we deploy PraaS control plane on a different VM.

(1) First, we need to create the Docker image and push it to the AWS ECR registry. Run `./examples/builder.py latex-editor python`.
The registry needs to be created before pushing the image.
```
docker tag spcleth/praas-examples:latex-editor-python <aws-id>.dkr.ecr.us-east-1.amazonaws.com/latex-fargate:latex-editor-python
docker push <aws-id>.dkr.ecr.us-east-1.amazonaws.com/latex-fargate:latex-editor-python
```

The reference container is `spcleth/praas-examples:latex-editor-python-v1`.

(2) Create a Fargate task on AWS with the specified image. Update the config file with the Fargate task definition in `cloud_resource_name`.

(3) Deploy control plane: `./build/control-plane/bin/control_plane --config control_plane.json`. Update the config file with control plane IP. The `fargate.json` needs the following values: `cluster_name`, `subnet`.

(4) Run the benchmark: `<build>/benchmarks/latex-service/benchmarker_latex_{mode} configs/{mode}_{size}.json` where `mode` is `editing` or `compiling`. Run the benchmarker from artifact, as it needs to find the input files.
