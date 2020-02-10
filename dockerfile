#Download base image ubuntu (latest version)
FROM ubuntu:latest

# Update Software repository
RUN apt update

# Add required dependencies
RUN apt-get -y install git httrack cmake automake pkg-config python3 python3-venv python3-pip libboost-all-dev openjdk-8-jdk liblzma-dev time poppler-utils curl wget locales

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
    && pip3 install --upgrade pip     \
    && pip3 install -r requirements.txt     \
    && pip3 install -r bicleaner/requirements.txt     \
    && pip3 install https://github.com/kpu/kenlm/archive/master.zip --install-option="--max_order 7" \
    && pip3 install -r bifixer/requirements.txt

# Installing giawarc
RUN wget -O go.tgz https://dl.google.com/go/go1.13.7.linux-amd64.tar.gz \
    && tar -C /usr/local -xzf go.tgz \
    && rm go.tgz \
    && export PATH="/usr/local/go/bin:$PATH" \
    && go version
ENV GOPATH /root/go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH

RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 777 "$GOPATH"
RUN go get github.com/paracrawl/giawarc/...
    

    

# Installing protobuf and cld3
    RUN apt-get install -y autoconf automake libtool curl make g++ unzip \
    && wget https://github.com/protocolbuffers/protobuf/releases/download/v3.11.3/protobuf-all-3.11.3.tar.gz \
    && tar -zxvf protobuf-all-3.11.3.tar.gz \
    && cd protobuf-3.11.3 \
    && ./configure \
    && make \
    && make check \
    && make install \
    && ldconfig \
    && pip3 install Cython \
    && pip3 install pycld3

# Installing bitextor
RUN cd /opt/bitextor     \
    && ./autogen.sh     \
    && make     \
    && make install

CMD /bin/bash
