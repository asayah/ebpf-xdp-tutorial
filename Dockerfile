
# build and push this to your repo, mine is asayah/xdp-tutorial-base

FROM ubuntu:20.04

RUN apt update
RUN apt-get --assume-yes install clang llvm libelf-dev libpcap-dev gcc-multilib build-essential make linux-tools-5.15.0-1022-gcp curl iputils-ping

ENTRYPOINT ["tail", "-f", "/dev/null"]