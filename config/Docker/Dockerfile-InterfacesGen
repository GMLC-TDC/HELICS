# This Dockerfile creates an image with the tools needed to generate the swig interfaces for HELICS

FROM fedora:31 as builder

# Installs dependencies needed for building, with headers needed for development
RUN dnf install g++ git autoconf libtool shtool make pcre-devel byacc -y

WORKDIR /root

RUN git clone --recurse-submodules --branch matlab https://github.com/jaeandersson/swig.git
RUN mkdir swig-matlab && cd swig && sh autogen.sh && ./configure --prefix=/root/swig-matlab 
RUN cd swig && make -j4 && make install


# Create an image for generating HELICS interfaces
FROM fedora:31 as Hbase

# Install the tools for building
RUN dnf install g++ git cmake make swig -y  && dnf clean all

# Copy the swig-matlab install from the builder image stage
COPY --from=builder /root/swig-matlab /root/swig-matlab
