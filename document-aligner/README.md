# Usage
```
Usage: ./dalign TRANSLATED-TOKENS ENGLISH-TOKENS

Additional options:
  --help                  produce help message
  --df-sample-rate arg    set sample rate to every n-th document (default: 1)
  --threads arg           set number of threads (default: all)
  --threshold arg         set score threshold (default: 0.1)
  --translated-tokens arg set input filename
  --english-tokens arg    set input filename
```

It is advisable to pass in --df-sample-rate to reduce start-up time and memory
usage for the document frequency part of TFIDF. 1 indicates that every document
will be read while 4 would mean that one of every four documents will be added
to the DF.

# Input
Two files (gzip-compressed or plain text) with on each line a single base64-
encoded list of tokens (separated by whitespace).

# Output
For each alignment score that is greater or equal to the threshold it prints the
score, and the indexes (starting with 1) of the documents in TRANSLATED-TOKENS
and ENGLISH-TOKENS, separated by tabs to STDOUT.

# Building on CSD3
```
module load gcc
module load cmake
```

Compile boost (available boost seems to not link properly).
```
wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz
tar -xzf boost_1_72_0.tar.gz
cd boost_1_72_0.tar.gz
./bootstrap.sh --prefix=$HOME/.local
./b2 install
```

Compile dalign with the up-to-date Boost
```
cd bitextor/document-aligner
mkdir build
cd build
cmake -D Boost_DIR=$HOME/.local/lib/cmake/Boost-1.72.0 ..
make -j4
```

Now you should have a `bin/dalign` in your build directory. Note that it is
linked to your custom Boost which makes it a bit less portable.