# Use an official Python runtime as a parent image
FROM ubuntu:trusty

# Set the working directory to /app
WORKDIR /build

# Install any needed packages
RUN apt-get -qq update && apt-get install -y --no-install-recommends \
        mercurial \
        wget \
        bzip2 \
        libbz2-dev \
        make \
        curl \
        cpuid \
        pkg-config \
        software-properties-common \
        binutils \
        libasan0 \
        libatomic1 \
        libcloog-isl4 \
        libisl10 \
        libitm1 \
        libquadmath0 \
        libtimedate-perl \
        libtsan0 \
        patch \
        zlib1g-dev \
        libc6-dev \
        cmake3;

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test;
RUN add-apt-repository -y ppa:nschloe/boost-nightly
RUN add-apt-repository -y ppa:mc3man/trusty-media;
RUN add-apt-repository -y ppa:nschloe/boost-nightly

RUN apt-get -qq update;

# Run app.py when the container launches
CMD ["bash"]
