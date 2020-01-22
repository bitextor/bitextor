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
import base64
from collections import Counter
from functools import partial
from multiprocessing import Pool

from numpy import float32
from scipy.sparse import vstack as sp_vstack
from scipy.sparse import csr_matrix, lil_matrix
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


def base64decode(text):
    return base64.b64decode(text.strip()).decode("utf-8").strip()


class ExtractionMapper(object):

    def __init__(self, extraction_function=None):
        self.ef = extraction_function

    def extract_single(self, b64page):
        return self.ef(base64decode(b64page))

    def extract_single_in_batches(self, b64pages):
        counter = Counter()
        for b64page in b64pages:
            counter.update(set(self.extract_single(b64page)))
        return counter


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
        self.total_ndocs = 0
        self.total_ndocs_sl = 0
        self.total_ndocs_tl = 0
        self.nonempty_ndocs = 0
        self.nonempty_ndocs_sl = 0
        self.nonempty_ndocs_tl = 0
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

    @staticmethod
    def iterate_corpus_in_batches(text_file, batch_size=10000):
        pages = []
        for page in text_file:
            if len(pages) < batch_size:
                pages.append(page)
            if len(pages) == batch_size:
                yield pages
                pages = []
        if len(pages) != 0:
            yield pages

    # Given all source and target corpus file objects, counts how many times a word is found in different documents
    # (source and target together, given that target is translated into source language) (AKA, idf)
    def estimate_idf(self, source_corpus, target_corpus, jobs=1, batch_size=10000):
        counts = Counter()

        def count_ngrams(corpus):
            # start = time.time()
            total = 0
            nonempty = 0
            if jobs == 1:
                for page in corpus:
                    total += 1
                    if page.strip():
                        nonempty += 1
                        counts.update(set(self.ef.extract_single(page)))
            else:
                pool = Pool(jobs - 1)
                for pages in self.iterate_corpus_in_batches(corpus, batch_size):
                    total += len(pages)
                    pages = filter(pages)  # removes empty pages
                    nonempty += len(pages)
                    pool.apply_async(self.ef.extract_single_in_batches, args=(pages,), callback=counts.update)
                pool.close()
                pool.join()
            return total, nonempty
            # end = time.time()
            # sys.stderr.write("completed estimate_idf in {0:.5f}\n".format(end - start))

        self.total_ndocs_sl, self.nonempty_ndocs_sl = count_ngrams(source_corpus)
        self.total_ndocs_tl, self.nonempty_ndocs_tl = count_ngrams(target_corpus)
        self.total_ndocs = self.total_ndocs_sl + self.total_ndocs_tl
        self.nonempty_ndocs = self.nonempty_ndocs_sl + self.nonempty_ndocs_tl
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
                idf = math.log(self.nonempty_ndocs / docs_with_term)
            elif self.idf_smooth == 2:
                idf = math.log(self.nonempty_ndocs / (1 + docs_with_term))
            elif self.idf_smooth == 3:
                idf = math.log(self.max_count / (1 + docs_with_term))
            elif self.idf_smooth == 4:
                if self.nonempty_ndocs > docs_with_term:
                    idf = math.log((self.nonempty_ndocs - docs_with_term) / docs_with_term)
                else:
                    idf = 0
            elif self.idf_smooth == 5:
                idf = math.log(self.nonempty_ndocs / (docs_with_term + 1))

            self.term2idf[term] = idf
            self.term2idx[term] = len(self.term2idx)

        # sys.stderr.write("{0} terms, {1} ignored\n".format(
        #    len(self.term2idx), len(self.ignored_terms)))

    def get_tf(self, count, local_max_count, local_sum):
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
        return tf

    def process_documents(self, start_doc_idx, pages):
        results = {}
        doc_idx = start_doc_idx
        for page in pages:
            counts = Counter(self.ef.extract_single(page))
            if not counts:
                doc_idx += 1
                continue
            results[doc_idx] = [], []
            local_max_count = float(max(counts.values()))
            local_sum = float(sum(counts.values()))
            for ngram, count in counts.items():
                if ngram not in self.term2idx:
                    # if ngram not in self.ignored_terms:
                    # sys.stderr.write("unknown ngram: %s\n" % ngram)
                    continue
                idf = self.term2idf[ngram]
                idx = self.term2idx[ngram]
                tf = self.get_tf(count, local_max_count, local_sum)
                results[doc_idx][0].append(idx)
                results[doc_idx][1].append(tf * idf)
            doc_idx = doc_idx + 1
        return results

    # Given a corpus file object and the number of documents it contains, counts word frequencies in each document (tf),
    # returning the resulting tf-idf matrix
    def extract(self, corpus_file, lencorpus, jobs=1, batch_size=10000):
        m = lil_matrix((lencorpus, len(self.term2idx)), dtype=float32)
        doc_idx = 0

        def cb(results):
            for doc_idx in results:
                m.rows[doc_idx] = results[doc_idx][0]
                m.data[doc_idx] = results[doc_idx][1]

        def err_cb(error):
            sys.stderr.write(str(error) + "\n")

        # start = time.time()
        if jobs == 1:
            for page in corpus_file:
                counts = Counter(self.ef.extract_single(page))
                if not counts:
                    doc_idx += 1
                    continue
                local_max_count = float(max(counts.values()))
                local_sum = float(sum(counts.values()))
                for ngram, count in counts.items():
                    if ngram not in self.term2idx:
                        continue
                    idf = self.term2idf[ngram]
                    idx = self.term2idx[ngram]
                    tf = self.get_tf(count, local_max_count, local_sum)
                    m[doc_idx, idx] = tf * idf
                doc_idx += 1
        else:
            pool = Pool(jobs - 1)
            for pages in self.iterate_corpus_in_batches(corpus_file, batch_size):
                pool.apply_async(self.process_documents, args=(doc_idx, pages,), callback=cb, error_callback=err_cb)
                doc_idx += len(pages)
            pool.close()
            pool.join()
        # end = time.time()
        # sys.stderr.write("tfidf took {0:.5f}\n".format(end-start))

        m = csr_matrix(m, dtype=float32)
        return m


class CosineDistanceScorer(object):

    def __init__(self, extraction_mapper, min_count,
                 smooth=0, threshold=0.1, batch_size=10000, jobs=1):
        self.name = "Cosine Distance Scorer"
        self.vector_extractor = DocumentVectorExtractor(
            extraction_mapper=extraction_mapper, min_count=min_count,
            smooth=smooth)
        self.threshold = threshold
        self.batch_size = batch_size
        self.jobs = jobs

    def partial_pairwise_distance(self, normalized_y_csr, x_csr_batch):
        normalize(x_csr_batch, copy=False)
        part_csr = x_csr_batch * normalized_y_csr.T
        part_csr.data[part_csr.data < self.threshold] = 0
        part_csr.eliminate_zeros()
        return part_csr

    def batched_pairwise_distances(self, x_csr, y_csr):
        def get_row_batch(m, batch):
            for cols_step in range(math.ceil(m.shape[0] / batch)):
                yield m[cols_step * batch:(cols_step + 1) * batch]

        all_csr = None

        # start = time.time()
        if self.jobs == 1:
            normalize(y_csr, copy=False)
            normalize(x_csr, copy=False)
            all_csr = x_csr * y_csr.T
            all_csr.data[all_csr.data < self.threshold] = 0
            all_csr.eliminate_zeros()
        else:
            normalize(y_csr, copy=False)
            pool = Pool(self.jobs)
            results = pool.map(partial(self.partial_pairwise_distance, y_csr), get_row_batch(x_csr, self.batch_size))
            pool.close()
            pool.join()
            for r in results:
                if all_csr is None:
                    all_csr = csr_matrix(r, dtype=float32)
                else:
                    all_csr = sp_vstack((all_csr, r))
        # end = time.time()
        # sys.stderr.write("pairwise distances computed in {:.5f}\n".format(end-start))
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

        with open_xz_or_gzip_or_plain(source_filepath) as source_text_file, open_xz_or_gzip_or_plain(target_filepath) as target_text_file:
            # start = time.time()
            self.vector_extractor.estimate_idf(source_text_file, target_text_file, jobs=self.jobs, batch_size=self.batch_size)
            # sys.stderr.write(
            #    "IDF estimation took {0:.5f} seconds\n".format(time.time() - start))

        # start = time.time()
        # Calculate tf and obtain tf-idf
        with open_xz_or_gzip_or_plain(source_filepath) as source_text_file:
            source_matrix = self.vector_extractor.extract(source_text_file,
                                                          self.vector_extractor.total_ndocs_sl,
                                                          jobs=self.jobs,
                                                          batch_size=self.batch_size)
        with open_xz_or_gzip_or_plain(target_filepath) as target_text_file:
            target_matrix = self.vector_extractor.extract(target_text_file,
                                                          self.vector_extractor.total_ndocs_tl,
                                                          jobs=self.jobs,
                                                          batch_size=self.batch_size)
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
        return d
