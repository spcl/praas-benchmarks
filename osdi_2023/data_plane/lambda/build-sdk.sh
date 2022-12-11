rm -rf aws-lambda-cpp
git clone https://github.com/awslabs/aws-lambda-cpp.git
mkdir lambda_sdk_build
cd lambda_sdk_build
cmake ../aws-lambda-cpp -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=$(pwd)/../lambda_sdk_install
make && make install

