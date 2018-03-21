#!/usr/bin/env python3
## By Jon Dehdari, 2015, public domain
## Counts ngrams, including joined ngrams, from text corpus

import sys

ngram_order = 4
ngrams = []
for i in range(ngram_order):
    ngrams.append({})

for line in sys.stdin:
    line = line.rstrip()
    tokens = line.split()
    #tokens.insert(0, "<s>")
    #tokens.append("</s>")
    #print(tokens)
    len_tokens = len(tokens)

    for i in range(len_tokens):

        # i := leftmost position
        # j := rightmost position of current sub-ngram
        # k := rightmost position of all sub-ngrams

        k = len_tokens if i+ngram_order >= len_tokens else i + ngram_order
        #print("i=",i, "k=", k, tokens[i:k])

        # Build-up joined ngrams
        for j in range(i+1,k+1):
            joined_ngram = '_'.join(tokens[i:j])
            if (j+1 < k):
                if joined_ngram in ngrams[0]:
                    ngrams[0][joined_ngram] += 1
                else :
                    ngrams[0][joined_ngram] = 1

            #print("   j=",j, joined_ngram)

            # Process sub-ngrams
            num_subcuts = j - (i+1)
            while (num_subcuts >= 1):
                if ( (j == k) and (num_subcuts % 2)): # skip imbalanced subcuts
                    num_subcuts -= 1
                    continue
                subcut = ' '.join([ '_'.join(tokens[i:i+num_subcuts]), '_'.join(tokens[i+num_subcuts:j]) ])
                if (subcut in ngrams[1]):
                    ngrams[1][subcut] +=1
                else :
                    ngrams[1][subcut] = 1

                #print("        num_subcuts=", num_subcuts, "subcut=<<",subcut, ">>")
                num_subcuts -= 1

for i in range(ngram_order):
    print()
    for k, v in sorted(ngrams[i].items()):
        print(k, "\t", v, sep='')
