FROM ubuntu:latest as source

RUN apt-get update && apt-get install -y g++

ADD benchmarker_mq.cpp .
ADD benchmarker_uds.cpp .
ADD benchmarker_shmem.cpp .

RUN g++ -O3 benchmarker_uds.cpp -o benchmarker_uds && g++ -O3 benchmarker_mq.cpp -o benchmarker_mq && g++ -O3 benchmarker_shmem.cpp -o benchmarker_shmem

FROM ubuntu:latest

COPY --from=source /benchmarker_uds .
COPY --from=source /benchmarker_mq .
COPY --from=source /benchmarker_shmem .

ADD run.sh .

RUN apt-get update && apt-get install -y openssh-server

RUN mkdir /var/run/sshd

RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

EXPOSE 22

RUN echo 'ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIMEz7XGDajQi/cTgig1wS3Z+3a7fJKGQ2212IuXkWTXM mcopik@mcopik-PC' > ${HOME}/.ssh/authorized_keys

CMD ["/usr/sbin/sshd", "-D"]

