#  This file is part of Bitextor.
#
#  Bitextor is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bitextor is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bitextor.  If not, see <https://www.gnu.org/licenses/>.

import math
import sys
import time
import os
from collections import Counter
from functools import partial
from multiprocessing import Pool

from numpy import float32, isnan, clip
from scipy.sparse import vstack as sp_vstack
from scipy.sparse import csr_matrix, lil_matrix
from sklearn.metrics.pairwise import pairwise_distances
from sklearn.preprocessing import normalize

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../utils")
from common import open_xz_or_gzip_or_plain


# Given a list of words ('words') returns a list of all 'n'-grams
def _ngram_helper(words, n, hash_values):
    words = [w.strip() for w in words if w.strip()]
    ngrams = (" ".join(words[i:i + n]) for i in
              range(max(len(words) - n + 1, 1)))
    ngrams = [ng for ng in ngrams if ng.strip()]
    if hash_values:
        return map(hash, ngrams)
    return ngrams


# Given a tokenized document ('page'), returns its 'n'-grams
def ngrams_from_text(n, hash_values, ignore_set, page):
    segments = page.split("\n")
    words = []
    for s in segments:
        words.extend(s.split(' '))
    ngrams = _ngram_helper(words, n, hash_values)

    if ignore_set:
        return [ng for ng in ngrams if ng not in ignore_set]

    return ngrams


# Only extract_single is being used
class ExtractionMapper(object):

    def __init__(self, extraction_function=None):
        self.ef = extraction_function

    def extract(self, corpus, pool=None):
        if pool is not None:
            return pool.map(self.ef, corpus)
        return map(self.ef, corpus)

    def extract_single(self, page):
        return self.ef(page)

    def extract_single_batch_to_set(self, pages):
        counter = Counter()
        for page in pages:
            counter.update(set(self.extract_single(page)))
        return counter

    def extract_source(self, corpus):
        return self.extract(corpus)

    def extract_target(self, corpus):
        return self.extract(corpus)


class WordExtractor(ExtractionMapper):

    def __init__(self, n=1, hash_values=False, ignore_set=None):
        super(WordExtractor, self).__init__(
            extraction_function=partial(ngrams_from_text,
                                        n, hash_values, ignore_set))


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
        # sys.stderr.write("TF: {0}\nIDF: {1}\n".format(
        #    self.tf_smooth, self.idf_smooth))
        assert int(self.tf_smooth) in range(7)
        assert int(self.idf_smooth) in range(6)
        self.lda_dim = lda_dim

    # Yields the url and plain text of each document read from file, grouping by url
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

    def iterate_corpus_in_batches(self, openfile, batch_size=10000):
        pages = []
        urls = []
        for url, page in self.iterate_corpus(openfile):
            if len(urls) < batch_size:
                pages.append(page)
                urls.append(url)
            if len(urls) == batch_size:
                yield urls, pages
                pages = []
                urls = []
        if len(urls) != 0:
            yield urls, pages

    # Given all source and target corpus file objects, counts how many times a word is found in different documents
    # (source and target together, given that target is translated into source language) (AKA, idf)
    def estimate_idf(self, source_corpus, target_corpus, jobs=1, batch_size=10000):
        counts = Counter()
        self.ndocs = 0
        self.ndocs_sl = 0
        self.ndocs_tl = 0

        def count_ngrams(corpus, pool):
            # start = time.time()
            for _, pages in self.iterate_corpus_in_batches(corpus, batch_size):
                self.ndocs += len(pages)
                pool.apply_async(self.ef.extract_single_batch_to_set, args=(pages,), callback=counts.update)
            # end = time.time()
            # sys.stderr.write("completed count_ngrams in {0:.5f}\n".format(end - start))

        start = time.time()
        pool = Pool(jobs)
        count_ngrams(source_corpus, pool)
        self.ndocs_sl = self.ndocs
        count_ngrams(target_corpus, pool)
        self.ndocs_tl = self.ndocs - self.ndocs_sl
        pool.close()
        pool.join()
        end = time.time()
        sys.stderr.write("completed estimate_idf in {0:.5f}\n".format(end-start))
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

        # sys.stderr.write("{0} terms, {1} ignored\n".format(
        #    len(self.term2idx), len(self.ignored_terms)))

    def process_documents(self, start_doc_idx, pages):
        results = {}
        doc_idx = start_doc_idx
        for page in pages:
            counts = Counter(self.ef.extract_single(page))
            if not counts:
                continue
            results[doc_idx] = [],[]
            local_max_count = float(max(counts.values()))
            local_sum = float(sum(counts.values()))
            for ngram, count in counts.items():
                if ngram not in self.term2idx:
                    # if ngram not in self.ignored_terms:
                    # sys.stderr.write("unknown ngram: %s\n" % ngram)
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
                results[doc_idx][0].append(idx)
                results[doc_idx][1].append(tf*idf)
            doc_idx = doc_idx + 1
        return results

    # Given a corpus file object and the number of documents it contains, counts word frequencies in each document (tf),
    # returning the resulting tf-idf matrix and document urls
    def extract(self, corpus, lencorpus, jobs=1, batch_size=10000):
        m = lil_matrix((lencorpus, len(self.term2idx)), dtype=float32)
        doc_idx = 0
        url_list = []

        def cb(results):
            for doc_idx in results:
                m.rows[doc_idx] = results[doc_idx][0]
                m.data[doc_idx] = results[doc_idx][1]

        def err_cb(error):
            sys.stderr.write(str(error) + "\n")

        start = time.time()
        pool = Pool(jobs)
        for urls, pages in self.iterate_corpus_in_batches(corpus, batch_size):
            url_list.extend(urls)
            pool.apply_async(self.process_documents, args=(doc_idx, pages,), callback=cb, error_callback=err_cb)
            doc_idx += len(urls)
        pool.close()
        pool.join()
        end = time.time()
        sys.stderr.write("tfidf took {0:.5f}\n".format(end-start))

        m = csr_matrix(m, dtype=float32)
        return url_list, m


class CosineDistanceScorer(object):

    def __init__(self, extraction_mapper, min_count, metric='cosine',
                 smooth=0, threshold=0.1, batch_size=10000, jobs=1):
        self.name = "Cosine Distance Scorer"
        self.metric = metric
        self.vector_extractor = DocumentVectorExtractor(
            extraction_mapper=extraction_mapper, min_count=min_count,
            smooth=smooth)
        self.threshold = threshold
        self.batch_size = batch_size
        self.jobs = jobs

    def batched_pairwise_distances(self, x_csr, y_csr):
        def get_row_batch(m, batch):
            for cols_step in range(math.ceil(m.shape[0] / batch)):
                yield m[cols_step * batch:(cols_step + 1) * batch]

        start = time.time()

        normalize(x_csr, copy=False)
        normalize(y_csr, copy=False)
        all_csr = x_csr * y_csr.T
        all_csr.data *= -1
        all_csr.data += 1
        clip(all_csr.data, 0, 2, out=all_csr.data)
        all_csr.data = 1 - all_csr.data
        # clip(all_csr.data, -1, 1, all_csr.data)
        all_csr.data[all_csr.data < self.threshold] = 0
        all_csr.eliminate_zeros()

        # all_csr = None
        #
        # for idx, X_batch in enumerate(get_row_batch(x_csr, self.batch_size)):
        #     pd = 1 - pairwise_distances(X_batch, y_csr, metric=self.metric)
        #     pd[(isnan(pd)) | (pd < self.threshold)] = 0
        #
        #     if all_csr is None:
        #         all_csr = csr_matrix(pd, dtype=float32)
        #     else:
        #         all_csr = sp_vstack((all_csr, csr_matrix(pd, dtype=float32)))
        #
        # print(all_csr)
        end = time.time()
        sys.stderr.write("pairwise distances computed in {:.5f}\n".format(end-start))
        return all_csr

    def munge_file_path(self, filepath):
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
                # start = time.time()
                self.vector_extractor.estimate_idf(source_file, target_file, jobs=self.jobs, batch_size=self.batch_size)
                # sys.stderr.write(
                #    "IDF estimation took {0:.5f} seconds\n".format(time.time() - start))

        # start = time.time()
        # Calculate tf and obtain tf-idf with urls
        with open_xz_or_gzip_or_plain(source_filepath) as source_file:
            urls[0], source_matrix = self.vector_extractor.extract(source_file, self.vector_extractor.ndocs_sl, jobs=self.jobs, batch_size=self.batch_size)
        with open_xz_or_gzip_or_plain(target_filepath) as target_file:
            urls[1], target_matrix = self.vector_extractor.extract(target_file, self.vector_extractor.ndocs_tl, jobs=self.jobs, batch_size=self.batch_size)
        # sys.stderr.write(
        #    "Matrix extraction took {0:.5f} seconds\n".format(time.time() - start))
        # sys.stderr.write(str(source_matrix)+"\n"+str(target_matrix)+"\n")
        # start = time.time()
        del self.vector_extractor

        if source_matrix.getnnz() == 0 or target_matrix.getnnz() == 0:
            d = None
        else:
            d = self.batched_pairwise_distances(source_matrix, target_matrix)

        # sys.stderr.write(
        #    "Scoring took {0:.5f} seconds\n".format(time.time() - start))
        return urls, d
