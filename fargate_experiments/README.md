
A useful test container that allows to SSH to Fargate and execute arbitrary commands.

(1) Replace your SSH public key in Dockerfile (and even better, use AWS secrets). Build the container `docker build -f Dockerfile -t fargate-test:ssh .`

(2) Create ECR repository and push to AWS.

```
docker tag fargate-test:ssh <user>.dkr.ecr.us-east-1.amazonaws.com/test-fargate:ssh
aws ecr get-login-password --region us-east-1 | docker login --username AWS --password-stdin <user>.dkr.ecr.us-east-1.amazonaws.com
docker push <user>.dkr.ecr.us-east-1.amazonaws.com/test-fargate:ssh
```

(3) Create ECS task in Fargate mode. Map the port 22 to container, and make sure to edit task's security group to allow port 22 for inboud connections.

(4) Start the container, wait for public IP assignment and `ssh root@<addr>`
