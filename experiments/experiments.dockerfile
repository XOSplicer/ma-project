FROM ubuntu:22.04

RUN apt-get -y update && \
    DEBIAN_FRONTEND=noninteractive apt-get -y install tzdata && \
    apt-get install -y --no-install-recommends \
        bash \
        build-essential \
        cmake \
        curl \
        git \
        g++ \
        libboost-graph-dev \
        libcurl4-openssl-dev \
        libstdc++6 \
        libsqlite3-dev \
        libxml2 \
        libxml2-dev \
        libz3-dev \
        software-properties-common \
        sqlite3 \
        wget \
        zlib1g-dev

RUN curl https://sh.rustup.rs -sSf | bash -s -- -y --default-toolchain nightly-2022-08-08
ENV PATH=/root/.cargo/bin:$PATH

RUN mkdir /ws
COPY . /ws/
WORKDIR /ws
