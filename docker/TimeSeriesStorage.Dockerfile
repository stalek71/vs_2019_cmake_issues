# Docker file should be called from the root project folder, because it uses whole build tree during the multi stage build.
# It should be refactored to multiple independent projects with an introduction of own vcpkgs (see vcpkg-configuration.json file in root folder)

# To build it just use the command shown below (from the root folder). Don't overlook dot (.) at the end:
# docker build -t mesh/tsstorage -f docker/TimeSeriesStorage.Dockerfile .

# This is a command to clean build cache:
# docker builder prune

FROM mesh/build-base:latest as build
LABEL description="Build container - Mesh"

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

# make config & build
RUN mkdir out \
    && cd out \
    && cmake .. -DCMAKE_TOOLCHAIN_FILE=/tmp/vcpkg/scripts/buildsystems/vcpkg.cmake \
                -DVCPKG_TARGET_TRIPLET=x64-linux-musl \
                -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" \
                -DCMAKE_CXX_COMPILER:STRING="g++" \
    && cmake --build . --target TimeSeriesStorageSvc -j 8

# Copy to the final (second stage) image
FROM scratch as runtime
LABEL description="Mesh - TimeSeries Storage Service"

# Main files
COPY --from=build /mesh/out/src/Services/Storage/TimeSeriesStorageSvc/TimeSeriesStorageSvc /mesh/out/src/Services/Storage/TimeSeriesStorageSvc/TimeSeriesStorageSvc

# Runtime dependencies
COPY --from=build /usr/lib/libstdc++.so.6 /usr/lib/libstdc++.so.6
COPY --from=build /usr/lib/libgcc_s.so.1 /usr/lib/libgcc_s.so.1
COPY --from=build /lib/libc.musl-x86_64.so.1 /lib/libc.musl-x86_64.so.1
COPY --from=build /lib/ld-musl-x86_64.so.1 /lib/ld-musl-x86_64.so.1

# Service start
CMD ["/mesh/out/src/Services/Storage/TimeSeriesStorageSvc/TimeSeriesStorageSvc"]

# Service port
EXPOSE 8080