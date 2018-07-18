zipporahdir = $(prefix)/share/bitextor/zipporah

zipporah_DATA = apply_logistic.py generate-bow-xent tokenizer.perl truecase.perl

generate-bow-xent:
	g++ zipporah/tools/generate-bow-xent.cc -o generate-bow-xent -std=c++11
