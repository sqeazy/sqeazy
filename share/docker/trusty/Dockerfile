FROM sqy/trusty/base:latest
FROM sqy/trusty/encoders:latest as encoders
FROM sqy/trusty/utils:latest as utils

#COPY --from=base /usr/local /usr/local

COPY --from=encoders /usr/local /usr/local
COPY --from=encoders /usr/lib /usr/lib
COPY --from=encoders /usr/include /usr/include

COPY --from=utils /usr/local /usr/local

WORKDIR /build

# RUN apt-get -qq update && apt-get install -y --no-install-recommends \
#         libmpc-dev \
#         libgmp-dev \
#         libmpfr-dev ;

# RUN wget --no-check-certificate ftp://ftp.gwdg.de/pub/misc/gcc/releases/gcc-7.2.0/gcc-7.2.0.tar.xz && \
#     tar xf gcc-7.2.0.tar.xz && \
#     mkdir build-7.2.0 && \
#     cd build-7.2.0 && \
#      ../gcc-7.2.0/configure --enable-threads=posix --enable-checking=release --with-system-zlib --enable-__cxa_atexit --enable-languages=c,c++ --with-cpu=generic --disable-multilib  --disable-nls --disable-bootstrap && \
#     make -j8 && make install;

# build sqeazy
RUN git clone https://github.com/sqeazy/sqeazy.git

# Run app.py when the container launches
CMD ["bash"]
