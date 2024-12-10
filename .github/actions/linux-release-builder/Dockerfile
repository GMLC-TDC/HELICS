FROM phusion/holy-build-box-64:2.1.0

# Install a copy of git and the Boost headers
# curl -L follows redirects and -k ignores SSL certificate warnings
RUN curl -Lk -o boost.tar.gz https://sourceforge.net/projects/boost/files/boost/1.75.0/boost_1_75_0.tar.gz/download \
    && tar --strip-components=1 -xf boost.tar.gz boost_1_75_0/boost \
    && cp -R boost /usr/include/boost/ \
    && rm -rf boost \
    && rm boost.tar.gz

RUN curl -Lk -o cmake-install.sh https://github.com/Kitware/CMake/releases/download/v3.25.3/cmake-3.25.3-Linux-x86_64.sh \
    && chmod +x cmake-install.sh \
    && ./cmake-install.sh --prefix=/hbb --exclude-subdir --skip-license \
    && rm cmake-install.sh

# Make sure the install location for cmake is found
ENV PATH="/hbb/bin:${PATH}"

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
