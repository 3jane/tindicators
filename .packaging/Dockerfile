FROM ubuntu:19.04

RUN apt update
RUN apt install -y cmake gcc tcl libasan5

WORKDIR /tulipindicators

COPY indicators.tcl /tulipindicators/
COPY indicators /tulipindicators/indicators/
COPY utils /tulipindicators/utils/
COPY CMakeLists.txt /tulipindicators/
COPY fuzzer.c benchmark.c sample.c example2.c benchmark2.c smoke.c /tulipindicators/

RUN mkdir build_debug && cd build_debug && cmake .. -DCMAKE_BUILD_TYPE=debug && make -j
RUN mkdir build_release && cd build_release && cmake .. -DCMAKE_BUILD_TYPE=release && make -j

COPY tests /tulipindicators/tests/

# `docker run --cap-add SYS_PTRACE`
CMD cd build_debug && export LD_PRELOAD=libasan.so.5 && make check && cd ../build_release && ./benchmark2
