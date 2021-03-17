#Download base image ubuntu (latest version)
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

# Update Software repository
ENV RED '\033[0;31m'
ENV NC '\033[0m'
RUN echo -e "${RED}Updating Software repository${NC}"
RUN apt update && apt upgrade -y && apt autoremove -y

# Setting up docker user
RUN apt -y install sudo
RUN useradd -r -m -d /home/docker -s /bin/bash -g root -G sudo docker
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Add required dependencies
RUN echo -e "${RED}Installing core apt dependencies${NC}"
RUN apt-get -y install git cmake python3 python3-venv python3-pip libboost-all-dev curl wget pigz unzip time parallel bc
# warc2text
RUN echo -e "${RED}Installing warc2text apt dependencies${NC}"
RUN apt-get -y install libuchardet-dev libzip-dev
# pdf-extract
RUN echo -e "${RED}Installing pdf-extract apt dependencies${NC}"
RUN apt-get -y install openjdk-8-jdk poppler-utils
# biroamer
RUN echo -e "${RED}Installing biroamer apt dependencies${NC}"
RUN apt-get -y install libgoogle-perftools-dev libsparsehash-dev

# random utilities:
# not necessary for bitextor, but users might find this useful:
RUN apt-get -y install htop vim

# Support for UTF8
# RUN locale-gen en_US.UTF-8
# ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# Cloning bitextor
USER docker
RUN echo -e "${RED}Cloning bitextor${NC}"
RUN mkdir -p /home/docker
RUN cd /home/docker && git clone -b docker --recurse-submodules --depth 1 https://github.com/bitextor/bitextor.git

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
RUN echo -e "${RED}Installing pip dependencies${NC}"
WORKDIR /home/docker/bitextor
RUN sudo pip3 install --upgrade pip
RUN sudo pip3 install -r requirements.txt
RUN sudo pip3 install -r bicleaner/requirements.txt
RUN sudo pip3 install https://github.com/kpu/kenlm/archive/master.zip --install-option="--max_order 7"
RUN sudo pip3 install -r bifixer/requirements.txt
RUN sudo pip3 install -r biroamer/requirements.txt
RUN sudo python3 -m spacy download en_core_web_sm

# symlink python to python3
RUN sudo ln -sf /usr/bin/python3 /usr/bin/python
RUN sudo ln -sf /usr/bin/pip3 /usr/bin/pip


# linguacrawl is currently broken (alcazar is unavailable)
# RUN pip3 install git+https://github.com/transducens/linguacrawl.git

# Installing giashard
WORKDIR /home/docker
RUN echo -e "${RED}Installing golang${NC}"
RUN wget -O go.tgz https://dl.google.com/go/go1.16.2.linux-amd64.tar.gz
RUN sudo tar -C /usr/local -xzf go.tgz && rm go.tgz
ENV PATH "/usr/local/go/bin:$PATH"
RUN go version
ENV GOPATH /home/docker/go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH
RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 777 "$GOPATH"
RUN echo -e "${RED}Installing giashard${NC}"
RUN go get github.com/paracrawl/giashard/...

# Installing bitextor
RUN echo -e "${RED}Compiling bitextor${NC}"
WORKDIR /home/docker/bitextor
RUN mkdir -p build
WORKDIR /home/docker/bitextor/build
RUN cmake -DSKIP_MGIZA=ON -DSKIP_CLUSTERCAT=ON ..
RUN make -j

# Download Heritrix
RUN echo -e "${RED}Downloading heritrix${NC}"
WORKDIR /home/docker
RUN wget http://builds.archive.org/maven2/org/archive/heritrix/heritrix/3.4.0-SNAPSHOT/heritrix-3.4.0-SNAPSHOT-dist.zip
RUN unzip heritrix-3.4.0-SNAPSHOT-dist.zip && rm heritrix-3.4.0-SNAPSHOT-dist.zip

# docker run bitextor with execute bitextor.sh by default
# any arguments passed to `docker run bitextor` command will be passed to bitextor.sh
ENTRYPOINT /home/docker/bitextor/bitextor.sh

# to execute interactive shell instead use:
# docker run -it --entrypoint /bin/bash bitextor
