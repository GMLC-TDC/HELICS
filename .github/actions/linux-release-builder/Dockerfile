ARG BASE_IMAGE=quay.io/pypa/manylinux_2_28_x86_64
FROM ${BASE_IMAGE}

# bash for ${var//./_} substitution; pipefail so a failed checksum stops the build
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Arguments for architecture specific downloads
ARG ARCH=x86_64
ARG BOOST_VERSION=1.92.0
ARG BOOST_SHA256=c4a3b310ddd2472416e091067166b0713be97c63f38c212c484ada022fd296ce
ARG CMAKE_VERSION=4.5.1

# Install git and other build essentials if missing
# Note: For musl support (Alpine), conditional logic for 'apk' instead of 'yum' would be needed here.
RUN yum install -y git && yum clean all

# Install the Boost headers from the official Boost archive, verifying the download
RUN BOOST_DIR="boost_${BOOST_VERSION//./_}" \
    && curl -fL -o boost.tar.gz "https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_DIR}.tar.gz" \
    && echo "${BOOST_SHA256}  boost.tar.gz" | sha256sum -c - \
    && tar --strip-components=1 -xf boost.tar.gz "${BOOST_DIR}/boost" \
    && mv boost /usr/include/boost \
    && rm boost.tar.gz

# Install CMake, verifying the installer against the release's published checksums
RUN CMAKE_INSTALLER="cmake-${CMAKE_VERSION}-linux-${ARCH}.sh" \
    && CMAKE_BASE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}" \
    && curl -fL -o "${CMAKE_INSTALLER}" "${CMAKE_BASE_URL}/${CMAKE_INSTALLER}" \
    && curl -fL -o cmake-SHA-256.txt "${CMAKE_BASE_URL}/cmake-${CMAKE_VERSION}-SHA-256.txt" \
    && grep " ${CMAKE_INSTALLER}\$" cmake-SHA-256.txt | sha256sum -c - \
    && chmod +x "${CMAKE_INSTALLER}" \
    && "./${CMAKE_INSTALLER}" --prefix=/usr/local --exclude-subdir --skip-license \
    && rm "${CMAKE_INSTALLER}" cmake-SHA-256.txt

# Make sure the install location for cmake is found
# /usr/local/bin should be in PATH by default

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
