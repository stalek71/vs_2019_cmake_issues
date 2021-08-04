# Docker file should be called from the root project folder, because it uses whole build tree during the build

# To build it use the command shown below (from the root folder) - remember about the dot (.) at the end:
# docker build -t mesh/build-base -f docker/BuildBase.Dockerfile .

FROM alpine:latest as build
LABEL description="Build container - Mesh"

# Core packages
RUN apk update && \
    apk add --no-cache \
    autoconf build-base binutils cmake curl file gcc g++ git libgcc libtool linux-headers make musl-dev libc6-compat tar unzip wget ninja zip pkgconf

# VCPKG
RUN cd /tmp \
    && git clone https://github.com/Microsoft/vcpkg.git \ 
    && cd vcpkg \
    && ./bootstrap-vcpkg.sh -useSystemBinaries

# Triplet for alpine
COPY docker/x64-linux-musl.cmake /tmp/vcpkg/triplets/

# Replace outdated cmake module and add support for PLUGIN param (can be removed when new version of cmake ships it)
COPY docker/cmake.replacements/Modules/FindProtobuf.cmake /usr/share/cmake/Modules/FindProtobuf.cmake

# Create mesh folder and switch to it
WORKDIR /mesh

# Copy folders
COPY cmake cmake
COPY src src
COPY tests tests

# Copy files
COPY CMakeLists.txt CMakePresets.json vcpkg.json vcpkg-configuration.json ./

# Environment vars
ENV VCPKG_ROOT=/tmp/vcpkg

# gRPC generator files
ENV PATH=/mesh/out/vcpkg_installed/x64-linux-musl/tools/grpc:/mesh/out/vcpkg_installed/x64-linux-musl/tools/protobuf:$PATH

# Build whole product with deps - it can be very good idea to define "empty" target to avoid whole build and install vcpkgs only
RUN mkdir out \
    && cd out \
    && cmake .. -DCMAKE_TOOLCHAIN_FILE=/tmp/vcpkg/scripts/buildsystems/vcpkg.cmake \
                -DVCPKG_TARGET_TRIPLET=x64-linux-musl \
                -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" \
                -DCMAKE_CXX_COMPILER:STRING="g++" \
    && make

# Remove all the source files - leave deps only
RUN rm -r /mesh

