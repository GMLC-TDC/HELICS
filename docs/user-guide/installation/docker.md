# Docker

## Requirements

- Docker 17.05 or higher

## Dockerfile

This `Dockerfile` will build and install HELICS in Ubuntu 22.04 with
Python support.

```dockerfile
FROM ubuntu:22.04 as builder

WORKDIR /root/develop

RUN apt update && apt install -y \
  libzmq5-dev python3-dev \
  libboost-all-dev \
  build-essential swig cmake git

RUN git clone --recurse-submodules \
  https://github.com/GMLC-TDC/HELICS.git helics

WORKDIR /root/develop/helics

RUN cmake \
  -DCMAKE_INSTALL_PREFIX=/helics \
  -DCMAKE_BUILD_TYPE=Release \
  -B build

RUN cmake --build build -j -t install


FROM ubuntu:22.04

COPY --from=builder /helics /usr/local/

ENV PYTHONPATH /usr/local/python

# Python must be installed after the PYTHONPATH is set above for it to
# recognize and import libhelicsSharedLib.so.
RUN apt update && apt install -y --no-install-recommends \
  libboost-filesystem1.74.0 libboost-program-options1.74.0 \
  libboost-test1.74.0 libzmq5 pip python3-dev

RUN pip install helics

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
