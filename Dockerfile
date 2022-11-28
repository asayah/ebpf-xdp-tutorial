FROM ubuntu:20.04

RUN apt update
RUN apt-get --assume-yes install clang llvm libelf-dev libpcap-dev gcc-multilib build-essential make linux-tools-5.15.0-1022-gcp curl

ENTRYPOINT ["tail", "-f", "/dev/null"]