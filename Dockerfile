ARG BASE_IMAGE=ubuntu:24.04

# Builder stage
FROM ${BASE_IMAGE} AS builder

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
    echo ">> Building CPU Mock version (No CUDA detected in Docker build environment)..." && \
    cmake -DENABLE_CUDA=OFF .. && \
    make

# Runtime stage
FROM ${BASE_IMAGE}

ENV DEBIAN_FRONTEND=noninteractive

# Create a non-root user and group
RUN addgroup --system appgroup && adduser --system --ingroup appgroup appuser

WORKDIR /home/appuser/app

# Copy the built executable from the builder stage
COPY --from=builder /app/build/my-server /home/appuser/app/my-server

# Change ownership of the application directory to the non-root user
RUN chown -R appuser:appgroup /home/appuser/app

USER appuser
EXPOSE 8000
CMD ["./my-server"]