ARG BASE_IMAGE=quay.io/pypa/manylinux_2_28_x86_64
FROM ${BASE_IMAGE}

# Arguments for architecture specific downloads
ARG ARCH=x86_64

# Install git and other build essentials if missing
# Note: For musl support (Alpine), conditional logic for 'apk' instead of 'yum' would be needed here.
RUN yum install -y git && yum clean all

# Install Boost
# Install a copy of git and the Boost headers
# curl -L follows redirects and -k ignores SSL certificate warnings
RUN curl -Lk -o boost.tar.gz https://sourceforge.net/projects/boost/files/boost/1.75.0/boost_1_75_0.tar.gz/download \
    && tar --strip-components=1 -xf boost.tar.gz boost_1_75_0/boost \
    && mv boost /usr/include/boost \
    && rm boost.tar.gz

# Install CMake
RUN if [ "$ARCH" = "aarch64" ]; then \
        CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v3.25.3/cmake-3.25.3-linux-aarch64.sh"; \
    else \
        CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v3.25.3/cmake-3.25.3-linux-x86_64.sh"; \
    fi \
    && curl -Lk -o cmake-install.sh "$CMAKE_URL" \
    && chmod +x cmake-install.sh \
    && ./cmake-install.sh --prefix=/usr/local --exclude-subdir --skip-license \
    && rm cmake-install.sh

# Make sure the install location for cmake is found
# /usr/local/bin should be in PATH by default

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
