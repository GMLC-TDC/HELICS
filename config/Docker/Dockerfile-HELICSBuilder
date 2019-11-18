FROM ubuntu:19.10 as builder

RUN apt update && apt install -y \
  libzmq5-dev\
  g++ \
  cmake git 
  
RUN apt-get install -y --no-install-recommends pkg-config libboost-dev make && apt-get clean

WORKDIR /root/develop
