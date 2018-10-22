
#ifndef FAST_BLEUALIGN_NGRAMS_H
#define FAST_BLEUALIGN_NGRAMS_H


#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <memory>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>


namespace ngram {

    struct NGramGeneral {

        typedef std::string ngram_type;

        virtual size_t size() const = 0;

        virtual ngram_type at(size_t pos) const = 0;

    };

    template<size_t N, typename... Args>
    struct NGram : public NGramGeneral {

        const size_t ngram_size = N;
        ngram_type data[N];

        NGram(Args... args) {
          size_t idx = 0;
          for (auto s: {args...}) {
            data[idx++] = s;
          }
        }

        size_t size() const {
          return ngram_size;
        }

        ngram_type at(size_t pos) const {
          if (pos >= ngram_size) {
            throw "Array index out of bounds!";
          }

          return data[pos];
        }

    };

    typedef NGram<4, std::string, std::string, std::string, std::string> quadragram;
    typedef NGram<3, std::string, std::string, std::string> trigram;
    typedef NGram<2, std::string, std::string> bigram;
    typedef NGram<1, std::string> unigram;


    struct NGramContainer {

        boost::shared_ptr<NGramGeneral> ng;

        NGramContainer(std::string a) : ng(boost::make_shared<unigram>(a)) {};

        NGramContainer(std::string a, std::string b) : ng(boost::make_shared<bigram>(a, b)) {};

        NGramContainer(std::string a, std::string b, std::string c) : ng(boost::make_shared<trigram>(a, b, c)) {};

        NGramContainer(std::string a, std::string b, std::string c, std::string d) : ng(
                boost::make_shared<quadragram>(a, b, c, d)) {};

        boost::shared_ptr<NGramGeneral> data() const {
          return ng;
        }

    };

    bool operator==(const NGramContainer &a, const NGramContainer &b);

    std::size_t hash_value(const NGramContainer &e);

    typedef boost::unordered_map<NGramContainer, size_t> ngram_map;


    class NGramCounter {

    public:

        NGramCounter(unsigned short n);

        ~NGramCounter() {};

        int get(const NGramContainer &key);

        ngram_map::iterator begin(size_t ngram) {
          return data.at(ngram - 1).begin();
        }

        ngram_map::iterator end(size_t ngram) {
          return data.at(ngram - 1).end();
        }

        void increment(const NGramContainer &ng);

        void process(std::vector<std::string> &tokens);

        inline void
        increment_helper(std::unique_ptr<std::vector<std::string>::iterator[]> &token_iterators, unsigned short ngram);

        size_t count_tokens();

        size_t count_frequencies() {
          return total_freq;
        }

        size_t processed() {
          return tokens_processed;
        }

        size_t get_map_index(const NGramContainer &key);

    private:

        const unsigned short ngram_size;
        size_t total_freq = 0;
        size_t tokens_processed = 0;
        std::vector<ngram_map> data;

    };

}


#endif //FAST_BLEUALIGN_NGRAMS_H
