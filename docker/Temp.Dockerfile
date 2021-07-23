FROM mesh:latest as build
FROM scratch as runtime

# Main files
#COPY --from=build /mesh/out/src/Libraries/Calculation/libCalculation.so libCalculation.so
#COPY --from=build /mesh/out/src/Services/Storage/Storage Storage

# Runtime dependencies
#COPY --from=build /usr/lib/libstdc++.so.6 libstdc++.so.6
#COPY --from=build /usr/lib/libgcc_s.so.1 libgcc_s.so.1
#COPY --from=build /lib/libc.musl-x86_64.so.1 libc.musl-x86_64.so.1
#COPY --from=build /lib/ld-musl-x86_64.so.1 ld-musl-x86_64.so.1

#ENV LD_LIBRARY_PATH=/


# Main files
COPY --from=build /mesh/out/src/Libraries/Calculation/libCalculation.so /mesh/out/src/Libraries/Calculation/libCalculation.so
COPY --from=build /mesh/out/src/Services/Storage/Storage /mesh/out/src/Services/Storage/Storage

# Runtime dependencies
COPY --from=build /usr/lib/libstdc++.so.6 /usr/lib/libstdc++.so.6
COPY --from=build /usr/lib/libgcc_s.so.1 /usr/lib/libgcc_s.so.1
COPY --from=build /lib/libc.musl-x86_64.so.1 /lib/libc.musl-x86_64.so.1
COPY --from=build /lib/ld-musl-x86_64.so.1 /lib/ld-musl-x86_64.so.1

# Service start
CMD ["/mesh/out/src/Services/Storage/Storage"]

