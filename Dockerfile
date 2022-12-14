#Download base image ubuntu (latest version)
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ARG j=1

# Update Software repository
ENV RED '\033[0;31m'
ENV NC '\033[0m'
RUN /bin/echo -e "${RED}Updating Software repository${NC}"
RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y

# Pretend that HOME is /home/docker because having things in / is awkward
RUN mkdir -p /home/docker
ENV HOME /home/docker

# Add required dependencies
RUN /bin/echo -e "${RED}Installing core apt dependencies${NC}"
RUN apt-get -y install git cmake python3 python3-venv python3-pip libboost-all-dev curl wget pigz unzip time parallel bc libhunspell-dev
# warc2text
RUN /bin/echo -e "${RED}Installing warc2text apt dependencies${NC}"
RUN apt-get -y install libuchardet-dev libzip-dev
# pdf-extract
RUN /bin/echo -e "${RED}Installing pdf-extract apt dependencies${NC}"
RUN apt-get -y install openjdk-8-jdk poppler-utils
# biroamer
RUN /bin/echo -e "${RED}Installing biroamer apt dependencies${NC}"
RUN apt-get -y install libgoogle-perftools-dev libsparsehash-dev
# fastspell
RUN /bin/echo -e "${RED}Installing fastspell apt dependencies${NC}"
RUN apt-get -y install hunspell-af hunspell-bg hunspell-bs hunspell-ca hunspell-cs hunspell-da hunspell-es hunspell-gl hunspell-hr hunspell-nl hunspell-no hunspell-pt-pt hunspell-sk hunspell-sl hunspell-sr


# random utilities:
# not necessary for bitextor, but users might find this useful:
RUN apt-get -y install htop vim

# symlink python to python3
RUN ln -sf /usr/bin/python3 /usr/bin/python
RUN ln -sf /usr/bin/pip3 /usr/bin/pip

# Support for UTF8
# RUN locale-gen en_US.UTF-8
# ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# Installing protobuf
RUN /bin/echo -e "${RED}Installing protobuf and CLD3${NC}"
WORKDIR /home/docker
RUN apt-get install -y autoconf automake libtool
RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.1/protobuf-all-3.19.1.tar.gz
RUN tar -zxvf protobuf-all-3.19.1.tar.gz
RUN rm protobuf-all-3.19.1.tar.gz
WORKDIR /home/docker/protobuf-3.19.1
RUN ./configure
RUN make -j $j
RUN make install
RUN ldconfig

# Installing giashard
RUN /bin/echo -e "${RED}Installing golang${NC}"
WORKDIR /home/docker
RUN wget -O go.tgz https://dl.google.com/go/go1.17.3.linux-amd64.tar.gz
RUN tar -C /usr/local -xzf go.tgz && rm go.tgz
ENV PATH "/usr/local/go/bin:$PATH"
RUN go version
ENV GOPATH /home/docker/go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH
RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 777 "$GOPATH"
RUN /bin/echo -e "${RED}Installing giashard${NC}"
RUN go install github.com/paracrawl/giashard/cmd/giashard@latest

# Download Heritrix
RUN /bin/echo -e "${RED}Downloading heritrix${NC}"
WORKDIR /home/docker
RUN wget https://repo1.maven.org/maven2/org/archive/heritrix/heritrix/3.4.0-20210923/heritrix-3.4.0-20210923-dist.zip
RUN unzip heritrix-3.4.0-20210923-dist.zip && rm heritrix-3.4.0-20210923-dist.zip

# Cloning bitextor
RUN /bin/echo -e "${RED}Cloning bitextor${NC}"
WORKDIR /home/docker
COPY ./ bitextor/

# Installing bitextor dependencies
RUN /bin/echo -e "${RED}Installing pip dependencies${NC}"
WORKDIR /home/docker/bitextor
RUN pip3 install --upgrade pip
## bitextor
RUN pip3 install .[all]
## bicleaner
RUN pip3 install ./third_party/bicleaner
RUN pip3 install ./third_party/bicleaner-ai
RUN pip3 install ./third_party/kenlm --install-option="--max_order=7"
##  bifixer
RUN pip3 install ./third_party/bifixer
## biroamer
RUN pip3 install ./third_party/biroamer
RUN python3 -c "from flair.models import SequenceTagger; SequenceTagger.load('flair/ner-english-fast')"
## neural
RUN pip3 install ./third_party/neural-document-aligner
RUN pip3 install ./third_party/vecalign
## cld3
RUN pip3 install Cython
RUN pip3 install pycld3


# Installing bitextor
RUN /bin/echo -e "${RED}Compiling bitextor${NC}"
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
