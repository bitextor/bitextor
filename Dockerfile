#Download base image ubuntu (latest version)
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ARG CONFIGFILE
ARG JOBS=1

# Update Software repository
ENV RED '\033[0;31m'
ENV NC '\033[0m'
RUN echo "${RED} Updating Software repository${NC}"
RUN apt update && apt upgrade -y && apt autoremove -y

# Add required dependencies
RUN echo -e "${RED} Installing core apt dependencies${NC}"
RUN apt-get -y install git cmake python3 python3-venv python3-pip libboost-all-dev curl wget pigz unzip
# warc2text
RUN echo -e "${RED} Installing warc2text apt dependencies${NC}"
RUN apt-get -y install libuchardet-dev libzip-dev
# pdf-extract
RUN echo -e "${RED} Installing pdf-extract apt dependencies${NC}"
RUN apt-get -y install openjdk-8-jdk poppler-utils
# biroamer
RUN echo -e "${RED} Installing biroamer apt dependencies${NC}"
RUN apt-get -y install libgoogle-perftools-dev libsparsehash-dev

# Support for UTF8
# RUN locale-gen en_US.UTF-8
# ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# Cloning bitextor
RUN echo -e "${RED} Cloning bitextor${NC}"
RUN cd /opt && git clone -b remove-autotools --recurse-submodules https://github.com/bitextor/bitextor.git

# Installing protobuf and cld3
## RUN echo -e "${RED} Installing protobuf and CLD3${NC}"
## RUN apt-get install -y autoconf automake libtool curl make g++ unzip \
## && wget https://github.com/protocolbuffers/protobuf/releases/download/v3.10.1/protobuf-all-3.10.1.tar.gz \
## && tar -zxvf protobuf-all-3.10.1.tar.gz \
## && cd protobuf-3.10.1 \
## && ./configure \
## && make \
## && make check \
## && make install \
## && ldconfig \
## && pip3 install Cython \
## && pip3 install pycld3

# Installing bitextor and bicleaner dependencies
RUN echo -e "${RED} Installing pip dependencies${NC}"
WORKDIR /opt/bitextor
RUN pip3 install --upgrade pip
RUN pip3 install -r requirements.txt
RUN pip3 install -r bicleaner/requirements.txt
RUN pip3 install https://github.com/kpu/kenlm/archive/master.zip --install-option="--max_order 7" 
RUN pip3 install -r bifixer/requirements.txt
RUN pip3 install -r biroamer/requirements.txt
RUN python3 -m spacy download en_core_web_sm

# linguacrawl is currently broken (alcazar is unavailable)
# RUN pip3 install git+https://github.com/transducens/linguacrawl.git

# Installing giashard
WORKDIR /
RUN echo -e "${RED} Installing golang and giashard${NC}"
RUN wget -O go.tgz https://dl.google.com/go/go1.16.2.linux-amd64.tar.gz \
    && tar -C /usr/local -xzf go.tgz \
    && rm go.tgz \
    && export PATH="/usr/local/go/bin:$PATH" \
    && go version
ENV GOPATH /root/go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH

RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 777 "$GOPATH"
RUN go get github.com/paracrawl/giashard/...
    
# Installing bitextor
RUN echo -e "${RED} Compiling bitextor ${NC}"
RUN cd /opt/bitextor \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j

# Download Heritrix
RUN echo -e "${RED} Downloading heritrix${NC}"
WORKDIR /opt
RUN wget http://builds.archive.org/maven2/org/archive/heritrix/heritrix/3.4.0-SNAPSHOT/heritrix-3.4.0-SNAPSHOT-dist.zip
RUN unzip heritrix-3.4.0-SNAPSHOT-dist.zip

CMD /opt/bitextor/bitextor.sh -s $CONFIGFILE -j $JOBS
