#Download base image ubuntu (latest version)
FROM ubuntu:latest

# Update Software repository
RUN apt update

# Add required dependencies
RUN apt-get -y install git httrack libboost-all-dev cmake automake pkg-config python3 python3-pip openjdk-8-jdk liblzma-dev time poppler-utils curl wget locales

# Support for UTF8
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# Cloning bitextor
RUN cd /opt     \
    && git clone --recurse-submodules https://github.com/bitextor/bitextor.git     \
    && cd bitextor     \
    && git submodule update --init --recursive

# Installing bitextor and bicleaner dependencies
RUN cd /opt/bitextor     \
    && pip3 install -r requirements.txt     \
    && pip3 install -r bicleaner/requirements.txt     \
    && pip3 install https://github.com/kpu/kenlm/archive/master.zip --install-option="--max_order 7" \
    && pip3 install -r bifixer/requirements.txt

# Installing giawarc
RUN snap install go \
    && go get github.com/paracrawl/giawarc/...

# Installing protobuf and cld3
    RUN apt-get install -y autoconf automake libtool curl make g++ unzip \
    && wget https://github.com/protocolbuffers/protobuf/releases/download/v3.9.1/protobuf-all-3.9.1.tar.gz \
    && tar -zxvf protobuf-all-3.9.1.tar.gz \
    && cd protobuf-3.9.1 \
    && ./configure \
    && make \
    && make check \
    && make install \
    && ldconfig \
    && pip3 install Cython \
    && pip3 install git+https://github.com/iamthebot/cld3

# Installing bitextor
RUN cd /opt/bitextor     \
    && ./autogen.sh     \
    && make     \
    && make install

CMD /bin/bash
