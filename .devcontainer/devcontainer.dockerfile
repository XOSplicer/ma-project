FROM ubuntu:22.04

ARG LLVM_INSTALL_DIR="/usr/local/llvm-14"

RUN apt-get -y update && \
    DEBIAN_FRONTEND=noninteractive apt-get -y install tzdata && \
    apt-get install -y --no-install-recommends \
        bash \
        bear \
        build-essential \
        cmake \
        curl \
        dirmngr \
        doxygen \
        g++ \
        gdb \
        git \
        gpg \
        graphviz \
        libboost-graph-dev \
        libcurl4-openssl-dev \
        libedit-dev \
        libncurses5-dev \
        libncursesw5-dev \
        libomp-dev \
        libsqlite3-dev \
        libstdc++6 \
        libxml2 \
        libxml2-dev \
        libz3-dev \
        ninja-build \
        python3 \
        python3-pip \
        python3-sphinx \
        software-properties-common \
        sqlite3 \
        sudo \
        swig \
        wget \
        zlib1g-dev

RUN mkdir -p /etc/apt/keyrings/ && \
    wget -O- https://apt.llvm.org/llvm-snapshot.gpg.key | \
        gpg --dearmor | \
        tee /etc/apt/keyrings/llvm-snapshot.gpg > /dev/null && \
    echo 'deb [signed-by=/etc/apt/keyrings/llvm-snapshot.gpg] http://apt.llvm.org/focal/ llvm-toolchain-focal-14 main' | \
        tee /etc/apt/sources.list.d/llvm-snapshot.list && \
    apt-get -y update && \
    apt-get -y install --no-install-recommends \
        clang-14 \
        clang-tidy-14 \
        libclang-14-dev \
        libclang-common-14-dev \
        libclang-cpp14-dev \
        libclang-rt-14-dev \
        libllvm14 \
        llvm-14-dev

RUN pip3 install Pygments pyyaml wllvm

RUN curl https://sh.rustup.rs -sSf | bash -s -- -y

RUN . "$HOME/.cargo/env" && /root/.cargo/bin/rustup toolchain add nightly-2022-05-18
RUN . "$HOME/.cargo/env" && /root/.cargo/bin/rustup toolchain add nightly-2022-06-27
RUN . "$HOME/.cargo/env" && /root/.cargo/bin/rustup toolchain add nightly-2022-07-09
RUN . "$HOME/.cargo/env" && /root/.cargo/bin/rustup toolchain add nightly-2022-08-08

ENV PATH=/root/.cargo/bin:$PATH

RUN cargo install cbindgen cargo-asm

ENV CC=/usr/bin/clang-14
ENV CXX=/usr/bin/clang++-14