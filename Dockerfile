#Download base image ubuntu (latest version)
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ARG j=1

# Update Software repository
ENV RED '\033[0;31m'
ENV NC '\033[0m'
RUN echo -e "${RED}Updating Software repository${NC}"
RUN apt update && apt upgrade -y && apt autoremove -y

# Pretend that HOME is /home/docker because having things in / is awkward
RUN mkdir -p /home/docker
ENV HOME /home/docker

# Add required dependencies
RUN echo -e "${RED}Installing core apt dependencies${NC}"
RUN apt -y install git cmake python3 python3-venv python3-pip libboost-all-dev curl wget pigz unzip time parallel bc
# warc2text
RUN echo -e "${RED}Installing warc2text apt dependencies${NC}"
RUN apt -y install libuchardet-dev libzip-dev
# pdf-extract
RUN echo -e "${RED}Installing pdf-extract apt dependencies${NC}"
RUN apt -y install openjdk-8-jdk poppler-utils
# biroamer
RUN echo -e "${RED}Installing biroamer apt dependencies${NC}"
RUN apt -y install libgoogle-perftools-dev libsparsehash-dev

# random utilities:
# not necessary for bitextor, but users might find this useful:
RUN apt -y install htop vim

# symlink python to python3
RUN ln -sf /usr/bin/python3 /usr/bin/python
RUN ln -sf /usr/bin/pip3 /usr/bin/pip

# Support for UTF8
# RUN locale-gen en_US.UTF-8
# ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# Installing protobuf
RUN echo -e "${RED}Installing protobuf and CLD3${NC}"
WORKDIR /home/docker
RUN apt install -y autoconf automake libtool
RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.1/protobuf-all-3.19.1.tar.gz
RUN tar -zxvf protobuf-all-3.19.1.tar.gz
RUN rm protobuf-all-3.19.1.tar.gz
WORKDIR /home/docker/protobuf-3.19.1
RUN ./configure
RUN make -j $j
RUN make install
RUN ldconfig

# Installing giashard
RUN echo -e "${RED}Installing golang${NC}"
WORKDIR /home/docker
RUN wget -O go.tgz https://dl.google.com/go/go1.17.3.linux-amd64.tar.gz
RUN tar -C /usr/local -xzf go.tgz && rm go.tgz
ENV PATH "/usr/local/go/bin:$PATH"
RUN go version
ENV GOPATH /home/docker/go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH
RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 777 "$GOPATH"
RUN echo -e "${RED}Installing giashard${NC}"
RUN go get github.com/paracrawl/giashard/...

# Download Heritrix
RUN echo -e "${RED}Downloading heritrix${NC}"
WORKDIR /home/docker
RUN wget https://repo1.maven.org/maven2/org/archive/heritrix/heritrix/3.4.0-20210923/heritrix-3.4.0-20210923-dist.zip
RUN unzip heritrix-3.4.0-20210923-dist.zip && rm heritrix-3.4.0-20210923-dist.zip

# Cloning bitextor
RUN echo -e "${RED}Cloning bitextor${NC}"
WORKDIR /home/docker
COPY ./ bitextor/

# Installing bitextor dependencies
RUN echo -e "${RED}Installing pip dependencies${NC}"
WORKDIR /home/docker/bitextor
RUN pip3 install --upgrade pip
## bitextor
RUN pip3 install .
## bicleaner
RUN pip3 install ./bicleaner
RUN pip3 install ./kenlm --install-option="--max_order=7"
##  bifixer
RUN pip3 install ./bifixer
## biroamer
RUN pip3 install ./biroamer
RUN python3 -m spacy download en_core_web_sm
## cld3
RUN pip3 install Cython
RUN pip3 install pycld3


# Installing bitextor
RUN echo -e "${RED}Compiling bitextor${NC}"
WORKDIR /home/docker/bitextor
RUN mkdir -p build
WORKDIR /home/docker/bitextor/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/usr ..
RUN make -j $j install

# docker run bitextor with execute bitextor.sh by default
# any arguments passed to `docker run bitextor` command will be passed to bitextor.sh
WORKDIR /home/docker
ENTRYPOINT ["bitextor"]
CMD ["-h"]

# to execute interactive shell instead use:
# docker run -it --entrypoint /bin/bash bitextor
