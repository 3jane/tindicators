FROM ubuntu:19.04

RUN apt update
RUN apt install -y cmake gcc tcl libasan5

RUN LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu/ && ldconfig

WORKDIR /tulipindicators

COPY indicators.tcl /tulipindicators/
COPY indicators /tulipindicators/indicators/
COPY utils /tulipindicators/utils/
COPY CMakeLists.txt /tulipindicators/
COPY fuzzer.c benchmark.c sample.c example2.c benchmark2.c smoke.c /tulipindicators/

RUN pwd && ls
RUN mkdir build_debug && cd build_debug && cmake .. -DCMAKE_BUILD_TYPE=debug && make -j
RUN mkdir build_release && cd build_release && cmake .. -DCMAKE_BUILD_TYPE=release && make -j

COPY tests /tulipindicators/tests/

CMD cd build_debug && export LD_PRELOAD=libasan.so.5 && ./benchmark2 && ./smoke && ./fuzzer
