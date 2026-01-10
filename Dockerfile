ARG BASE_IMAGE=ubuntu:24.04
FROM ${BASE_IMAGE}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    lcov \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
ARG OATPP_VERSION=1.3.0-latest
RUN git clone --branch ${OATPP_VERSION} https://github.com/oatpp/oatpp.git \
    && cd oatpp \
    && mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DOATPP_INSTALL=ON .. \
    && make install

WORKDIR /app
COPY . /app

RUN mkdir -p build && cd build && \
    # Cek apakah compiler NVCC (CUDA) ada?
    if command -v nvcc > /dev/null; then \
        echo ">> PRODUCTION MODE: CUDA Detected. Building with GPU support..." && \
        cmake -DENABLE_CUDA=ON ..; \
    else \
        echo ">> DEV MODE: No CUDA found. Building CPU Mock version..." && \
        cmake -DENABLE_CUDA=OFF ..; \
    fi && \
    make

EXPOSE 8000
CMD ["./build/my-server"]