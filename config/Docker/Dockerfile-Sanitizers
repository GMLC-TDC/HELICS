FROM ubuntu:19.10 as builder

RUN apt update && apt install -y \
  python3 \
  make clang cmake git wget subversion libc++-dev libc++abi-dev

WORKDIR /root/develop

RUN svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
RUN (cd llvm/projects && svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx)
RUN (cd llvm/projects && svn co http://llvm.org/svn/llvm-project/libcxxabi/trunk libcxxabi)

# Sets the number of threads for make to use
ARG MAKE_PARALLEL

#build the ASAN libraries
RUN  mkdir libcxx_asan_build && cd libcxx_asan_build &&  cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_SANITIZER=Address -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-stdlib=libc++ 
RUN cd libcxx_asan_build && make cxx -j${MAKE_PARALLEL:-2}

RUN mkdir -p /root/develop/libcxx_asan/include && cp -r /root/develop/libcxx_asan_build/include/* /root/develop/libcxx_asan/include/
RUN mkdir -p /root/develop/libcxx_asan/lib && cp -r /root/develop/libcxx_asan_build/lib/* /root/develop/libcxx_asan/lib/

#build the Memory sanitizer libraries
RUN  mkdir libcxx_msan_build && cd libcxx_msan_build &&  cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_SANITIZER=Memory -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-stdlib=libc++ 
RUN cd libcxx_msan_build && make cxx -j${MAKE_PARALLEL:-2}

RUN mkdir -p /root/develop/libcxx_msan/include && cp -r /root/develop/libcxx_msan_build/include/* /root/develop/libcxx_msan/include/
RUN mkdir -p /root/develop/libcxx_msan/lib && cp -r /root/develop/libcxx_msan_build/lib/* /root/develop/libcxx_msan/lib/

#build the tsan libraries
RUN  mkdir libcxx_tsan_build && cd libcxx_tsan_build &&  cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_SANITIZER=Thread -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-stdlib=libc++ 
RUN cd libcxx_tsan_build && make cxx -j${MAKE_PARALLEL:-2}

RUN mkdir -p /root/develop/libcxx_tsan/include && cp -r /root/develop/libcxx_tsan_build/include/* /root/develop/libcxx_tsan/include/
RUN mkdir -p /root/develop/libcxx_tsan/lib && cp -r /root/develop/libcxx_tsan_build/lib/* /root/develop/libcxx_tsan/lib/

FROM ubuntu:19.10

RUN apt update && apt install -y \
  libboost-dev \
  clang cmake make git && apt-get clean && ln -s /usr/bin/llvm-symbolizer-9 /usr/bin/llvm-symbolizer

#copy the built libraries from the builder images
COPY --from=builder /root/develop/libcxx_asan /root/develop/libcxx_asan
COPY --from=builder /root/develop/libcxx_tsan /root/develop/libcxx_tsan
COPY --from=builder /root/develop/libcxx_msan /root/develop/libcxx_msan

# copy scripts to build and run HELICS with the sanitizers
COPY helics_build_run_*.sh /root/develop/

ENV ASAN_CFLAGS="-fsanitize=undefined,address -stdlib=libc++ -L/root/develop/libcxx_asan/lib -lc++abi -I/root/develop/libcxx_asan/include -I/root/develop/libcxx_asan/include/c++/v1 -Wno-unused-command-line-argument -fno-omit-frame-pointer -g -O1 -Wl,-rpath=/root/develop/libcxx_asan/lib"

ENV MSAN_CFLAGS="-fsanitize=memory -stdlib=libc++ -L/root/develop/libcxx_msan/lib -lc++abi -I/root/develop/libcxx_msan/include -I/root/develop/libcxx_msan/include/c++/v1 -Wno-unused-command-line-argument -fno-omit-frame-pointer -g -O1 -Wl,-rpath=/root/develop/libcxx_msan/lib"

ENV TSAN_CFLAGS="-fsanitize=thread -stdlib=libc++ -L/root/develop/libcxx_tsan/lib -lc++abi -I/root/develop/libcxx_tsan/include -I/root/develop/libcxx_tsan/include/c++/v1 -Wno-unused-command-line-argument -g -O1 -Wl,-rpath=/root/develop/libcxx_tsan/lib"
