CXX = g++

CXXFLAGS = \
	-std=c++11 \
	-Wall \
	-g \
	-I../../preprocess\
	-L../../preprocess/build/lib\
	-lpreprocess_util

dalign: main.cpp document.cpp ngram.cpp base64.cpp murmur_hash.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
