# Docker

## Requirements

- Docker 17.05 or higher

## Dockerfile

This `Dockerfile` will build and install HELICS in Ubuntu 18.04 with
Python support.

```dockerfile
FROM ubuntu:18.04 as builder

RUN apt update && apt install -y \
  libboost-dev \
  libboost-filesystem-dev \
  libboost-program-options-dev \
  libboost-test-dev \
  libzmq5-dev python3-dev \
  build-essential swig cmake git

WORKDIR /root/develop

RUN git clone https://github.com/GMLC-TDC/HELICS.git helics

WORKDIR /root/develop/helics/build

RUN cmake \
  -DBUILD_PYTHON_INTERFACE=ON \
  -DPYTHON_INCLUDE_DIR=/usr/include/python3.6/ \
  -DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.6m.so \
  -DCMAKE_INSTALL_PREFIX=/helics \
  ..
RUN make -j8 && make install

FROM ubuntu:18.04

RUN apt update && apt install -y --no-install-recommends \
  libboost-filesystem1.65.1 libboost-program-options1.65.1 \
  libboost-test1.65.1 libzmq5

COPY --from=builder /helics /usr/local/

ENV PYTHONPATH /usr/local/python

# Python must be installed after the PYTHONPATH is set above for it to
# recognize and import libhelicsSharedLib.so.
RUN apt install -y --no-install-recommends python3-dev \
  && rm -rf /var/lib/apt/lists/*

CMD ["python3", "-c", "import helics; print(helics.helicsGetVersion())"]
```

## Build

To build the Docker image, run the following from the directory
containing the `Dockerfile`:

```bash
$ docker build -t helics .
```

## Run

To run the Docker image as a container, run the following:

```bash
$ docker run -it --rm helics
```

Doing so should print the version and exit.
