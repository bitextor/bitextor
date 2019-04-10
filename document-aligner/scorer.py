import math
import sys
import time
import os
from collections import Counter
from functools import partial

from numpy import float32, isnan
from scipy.sparse import vstack as sp_vstack
from scipy.sparse import csr_matrix, lil_matrix
from sklearn.metrics.pairwise import pairwise_distances
from external_processor import ExternalTextProcessor

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../utils")
from common import open_xz_or_gzip_or_plain

#Given a list of words ('words') returns a list of all 'n'-grams
def _ngram_helper(words, n, hash_values):
    words = [w.strip() for w in words if w.strip()]
    ngrams = (" ".join(words[i:i + n]) for i in
              range(max(len(words) - n + 1, 1)))
    ngrams = [ng for ng in ngrams if ng.strip()]
    if hash_values:
        return map(hash, ngrams)
    return ngrams

#Given a document ('page'), tokenizes it and return its 'n'-grams
def ngrams_from_text(n, hash_values, ignore_set, word_tokeniser_cmd, page):
    proc = ExternalTextProcessor(word_tokeniser_cmd.split(' '))
    segments = proc.process(page).split("\n")
#    segments = page.split("\n")
    words = []
    for s in segments:
        words.extend(s.split(' '))
    ngrams = _ngram_helper(words, n, hash_values)

    if ignore_set:
        return [ng for ng in ngrams if ng not in ignore_set]

    return ngrams

#Only extract_single is being used
class ExtractionMapper(object):

    def __init__(self, extraction_function=None):
        self.ef = extraction_function

    def extract(self, corpus, pool=None):
        if pool is not None:
            return pool.map(self.ef, corpus)
        return map(self.ef, corpus)

    def extract_single(self, page):
        return self.ef(page)

    def extract_source(self, corpus):
        return self.extract(corpus)

    def extract_target(self, corpus):
        return self.extract(corpus)


class WordExtractor(ExtractionMapper):

    def __init__(self, word_tokeniser_cmd, n=1, hash_values=False, ignore_set=None):
        super(WordExtractor, self).__init__(
            extraction_function=partial(ngrams_from_text,
                                        n, hash_values, ignore_set, word_tokeniser_cmd))


class DocumentVectorExtractor(object):

    def __init__(self, extraction_mapper,
                 min_count=1, max_count=1000,
                 smooth=0, lda_dim=0):
        self.min_term_count = min_count
        self.max_term_count = max_count
        self.ef = extraction_mapper
        self.ndocs = 0
        self.ndocs_sl = 0
        self.ndocs_tl = 0
        self.term2idf = {}
        self.term2idx = {}
        self.ignored_terms = set()
        self.max_count = 0
        self.tf_smooth = smooth // 6
        self.idf_smooth = smooth % 6
        #sys.stderr.write("TF: {0}\nIDF: {1}\n".format(
        #    self.tf_smooth, self.idf_smooth))
        assert int(self.tf_smooth) in range(7)
        assert int(self.idf_smooth) in range(6)
        self.lda_dim = lda_dim

    #Yields the url and plain text of each document read from file, grouping by url
    def iterate_corpus(self,openfile):
        prevurl=""
        prevtext=""
        for line in openfile:
            line_split = line.strip().split('\t', 1)
            if len(line_split) != 2:
                continue
            url, text = line_split
            if url == prevurl:
                prevtext = prevtext+"\n"+text
            elif prevurl == "":
                prevurl = url
                prevtext = text
            elif url != prevurl:
                yield prevurl, prevtext
                prevurl = url
                prevtext = text
        yield prevurl, prevtext
    #Given all source and target corpus file objects, counts how many times a word is found in different documents (source and target together, given that target is translated into source language) (AKA, idf)
    def estimate_idf(self, source_corpus, target_corpus):
        counts = Counter()
        self.ndocs = 0
        self.ndocs_sl = 0
        for url, page in self.iterate_corpus(source_corpus):
            counts.update(set(self.ef.extract_single(page)))
            self.ndocs += 1
            self.ndocs_sl += 1
        self.ndocs_tl = 0
        for url, page in self.iterate_corpus(target_corpus):
            counts.update(set(self.ef.extract_single(page)))
            self.ndocs += 1
            self.ndocs_tl += 1

        self.term2idf = {}
        self.term2idx = {}
        self.ignored_terms = set()
        self.max_count = max(counts.values())
        for term, docs_with_term in counts.items():
            if docs_with_term < self.min_term_count:
                self.ignored_terms.add(term)
                continue

            if docs_with_term > self.max_term_count:
                self.ignored_terms.add(term)
                continue

            idf = 1
            if self.idf_smooth == 0:
                idf = 1
            elif self.idf_smooth == 1:
                idf = math.log(self.ndocs / docs_with_term)
            elif self.idf_smooth == 2:
                idf = math.log(self.ndocs / (1 + docs_with_term))
            elif self.idf_smooth == 3:
                idf = math.log(self.max_count / (1 + docs_with_term))
            elif self.idf_smooth == 4:
                if self.ndocs > docs_with_term:
                    idf = math.log(
                        (self.ndocs - docs_with_term) / docs_with_term)
                else:
                    idf = 0
            elif self.idf_smooth == 5:
                idf = math.log(self.ndocs / (docs_with_term + 1))

            self.term2idf[term] = idf
            self.term2idx[term] = len(self.term2idx)

        #sys.stderr.write("{0} terms, {1} ignored\n".format(
        #    len(self.term2idx), len(self.ignored_terms)))
    #Given a corpus file object and the number of documents it contains, counts word frequencies in each document (tf), returning the resulting tf-idf matrix and document urls
    def extract(self, corpus, lencorpus):
        m = lil_matrix((lencorpus, len(self.term2idx)), dtype=float32)
        doc_idx = 0
        url_list=[]
        for url, page in self.iterate_corpus(corpus):
            url_list.append(url)
            counts = Counter(self.ef.extract_single(page))
            if not counts:
                continue
            local_max_count = float(max(counts.values()))
            local_sum = float(sum(counts.values()))
            for ngram, count in counts.items():
                if ngram not in self.term2idx:
                    #if ngram not in self.ignored_terms:
                        #sys.stderr.write("unknown ngram: %s\n" % ngram)
                    continue

                idf = self.term2idf[ngram]
                idx = self.term2idx[ngram]

                tf = 1
                if self.tf_smooth == 0:
                    tf = 1
                elif self.tf_smooth == 1:
                    tf = count
                elif self.tf_smooth == 2:
                    tf = math.log(1 + count)
                elif self.tf_smooth == 3:
                    tf = 0.5 + 0.5 * (count / local_max_count)
                elif self.tf_smooth == 4:
                    tf = count / local_max_count
                elif self.tf_smooth == 5:
                    tf = count / local_sum
                elif self.tf_smooth == 6:
                    tf = math.sqrt(count)
                tfidf = tf * idf
                m[doc_idx, idx] = tfidf
            doc_idx += 1

        m = csr_matrix(m, dtype=float32)
        return url_list,m


class CosineDistanceScorer(object):

    def __init__(self, extraction_mapper, min_count, metric='cosine',
                 smooth=0, threshold=0.1, batch_size=10000):
        self.name = "Cosine Distance Scorer"
        self.metric = metric
        self.vector_extractor = DocumentVectorExtractor(
            extraction_mapper=extraction_mapper, min_count=min_count,
            smooth=smooth)
        self.threshold = threshold
        self.batch_size = batch_size

    def batched_pairwise_distances(self, x_csr, y_csr):

        def get_row_batch(m, batch):
            for cols_step in range(math.ceil(m.shape[0] / batch)):
                yield m[cols_step * batch:(cols_step + 1) * batch]

        all_csr = None
        for idx, X_batch in enumerate(get_row_batch(x_csr, self.batch_size)):
            pd = 1 - pairwise_distances(X_batch, y_csr, metric=self.metric)
            pd[(isnan(pd)) | (pd < self.threshold)] = 0

            if all_csr is None:
                all_csr = csr_matrix(pd, dtype=float32)
            else:
                all_csr = sp_vstack((all_csr, csr_matrix(pd, dtype=float32)))

        return all_csr

    def munge_file_path(self,filepath):
        if os.path.isfile(filepath):
            return filepath
        if os.path.isfile(filepath + ".gz"):
            return filepath + ".gz"
        if os.path.isfile(filepath + ".xz"):
            return filepath + ".xz"
        if os.path.isfile(filepath + ".bz2"):
            return filepath + ".bz2"
    
        # return nothing. file does not exist
        return None

    def score(self, source_filepath, target_filepath):
        source_filepath = self.munge_file_path(source_filepath)
        target_filepath = self.munge_file_path(target_filepath)
        urls = [[],[]]

        with open_xz_or_gzip_or_plain(source_filepath) as source_file:
            with open_xz_or_gzip_or_plain(target_filepath) as target_file:
                #start = time.time()
                self.vector_extractor.estimate_idf(source_file, target_file)
                #sys.stderr.write(
                #    "IDF estimation took {0:.5f} seconds\n".format(time.time() - start))

        #start = time.time()
        #Calculate tf and obtain tf-idf with urls
        with open_xz_or_gzip_or_plain(source_filepath) as source_file:
            urls[0], source_matrix = self.vector_extractor.extract(source_file,self.vector_extractor.ndocs_sl)
        with open_xz_or_gzip_or_plain(target_filepath) as target_file:
            urls[1], target_matrix = self.vector_extractor.extract(target_file,self.vector_extractor.ndocs_tl)
        #sys.stderr.write(
        #    "Matrix extraction took {0:.5f} seconds\n".format(time.time() - start))
        #sys.stderr.write(str(source_matrix)+"\n"+str(target_matrix)+"\n")
        #start = time.time()
        del self.vector_extractor

        if source_matrix.getnnz() == 0 or target_matrix.getnnz() == 0:
            d = None
        else:
            d = self.batched_pairwise_distances(source_matrix, target_matrix)

        #sys.stderr.write(
        #    "Scoring took {0:.5f} seconds\n".format(time.time() - start))
        return urls,d
