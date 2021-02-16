FROM phusion/holy-build-box-64:2.1.0

# Install a copy of git and the Boost headers
# curl -L follows redirects and -k ignores SSL certificate warnings
RUN curl -Lk -o boost.tar.gz https://sourceforge.net/projects/boost/files/boost/1.72.0/boost_1_72_0.tar.gz/download \
    && tar --strip-components=1 -xf boost.tar.gz boost_1_72_0/boost \
    && cp -R boost /usr/include/boost/ \
    && rm -rf boost \
    && rm boost.tar.gz

# Make sure the install location for cmake is found
ENV PATH="/hbb/bin:${PATH}"

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
