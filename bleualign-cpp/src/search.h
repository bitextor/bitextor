
#ifndef FAST_BLEUALIGN_SEARCH_H
#define FAST_BLEUALIGN_SEARCH_H


#include "align.h"
#include "utils/common.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <boost/unordered_map.hpp>


namespace search {

    const static double max_score = 100;

    class Searcher {

    public:

        virtual void process(std::vector<utils::scoremap> &smap_list) = 0;

        virtual void extract_matches(utils::matches_vec &res) = 0;

    };

    class Dynamic : public Searcher {

    public:

        Dynamic(size_t rows, size_t cols);

        ~Dynamic() {};

        double *get_score(size_t r, size_t c);

        char *get_backpointer(size_t r, size_t c);

        void process(std::vector<utils::scoremap> &smap_list) override;

        void show();

        void extract_matches(utils::matches_vec &res) override;


    private:

        size_t rows = 0;
        size_t cols = 0;

        boost::unordered_map<utils::sizet_pair, double> alignments;
        std::unique_ptr<double[]> scores;
        std::unique_ptr<char[]> back_pointers;

    };


    class Munkres {

    public:

        Munkres(size_t rows, size_t cols, bool min_cost = true);

        ~Munkres() {};

        void process(std::vector<utils::scoremap> &smap_list);

        void process(const std::vector<double> &input_costs);

        void show();

        double *get_cost(size_t r, size_t c);

        int *get_mask(size_t r, size_t c);

        void run_step(short step = 1);

        void extract_matches(utils::matches_vec &res);


    private:

        bool transposed;

        void step1();

        void step2();

        size_t step3();

        bool step4(std::pair<size_t, size_t> &ref);

        bool find_zero(std::pair<size_t, size_t> &ref);

        size_t find_prime_in_row(size_t r);

        size_t find_star_in_row(size_t r);

        size_t find_star_in_col(size_t c);

        void step5(std::pair<size_t, size_t> path_data);

        void augment_path(size_t path_cout);

        void erase_primes();

        void step6();


        size_t rows = 0;
        size_t cols = 0;
        bool min_cost;

        std::unique_ptr<double[]> costs;
        std::unique_ptr<int[]> mask;
        std::unique_ptr<bool[]> row_cover;
        std::unique_ptr<bool[]> col_cover;
        std::unique_ptr<int[]> path0;
        std::unique_ptr<int[]> path1;

    };


    void FindMatches(utils::matches_vec &matches, std::vector<utils::scoremap> &scorelist, size_t translated_size,
                     size_t english_size, double threshold = 0);

    void FilterMatches(utils::matches_vec &matches, std::vector<utils::scoremap> &scorelist, double threshold = 0);


}

#endif //FAST_BLEUALIGN_SEARCH_H

