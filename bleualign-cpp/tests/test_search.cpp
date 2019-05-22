
#include "gtest/gtest.h"
#include "../src/search.h"
#include "../src/align.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>


using namespace search;


namespace {

    TEST(munkres, test_munkres1) {

      utils::matches_vec matches;
      utils::matches_vec expected = {
              utils::match(0, 0, 1, 1, 0.0f),
              utils::match(1, 1, 0, 0, 0.0f),
              utils::match(2, 2, 2, 2, 0.0f),
      };

      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;

      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(4., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(1., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(3., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(2., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(0., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(5., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(3., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(2., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(2., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      Munkres mm(3, 3);
      mm.process(scorelist);
      mm.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }

    }


    TEST(munkres, test_munkres2) {

      utils::matches_vec matches;
      utils::matches_vec expected = {
              utils::match(0, 0, 1, 1, 0.0f),
              utils::match(1, 1, 3, 3, 0.0f),
              utils::match(2, 2, 2, 2, 0.0f),
      };

      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;

      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(4., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(1., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(3., std::make_pair(2, dummy)));
      smap.insert(utils::scoremap::value_type(5., std::make_pair(3, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(2., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(0., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(5., std::make_pair(2, dummy)));
      smap.insert(utils::scoremap::value_type(0., std::make_pair(3, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(3., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(2., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(2., std::make_pair(2, dummy)));
      smap.insert(utils::scoremap::value_type(4., std::make_pair(3, dummy)));
      scorelist.push_back(smap);

      Munkres mm(3, 4);
      mm.process(scorelist);
      mm.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }

    }


    TEST(munkres, test_munkres3) {

      utils::matches_vec matches;
      utils::matches_vec expected = {
              utils::match(1, 1, 1, 1, 0.0f),
              utils::match(2, 2, 2, 2, 0.0f),
              utils::match(3, 3, 0, 0, 0.0f),
      };

      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;

      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(4., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(1., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(3., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(2., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(0., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(5., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(3., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(2., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(2., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(1., std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(1., std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(1., std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      Munkres mm(4, 3);
      mm.process(scorelist);
      mm.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }

    }


    TEST(munkres, test_munkres4) {

      utils::matches_vec matches;
      std::vector<double> costs = {6, 6, 3, 4, 4, 9, 2, 4, 4, 3, 2, 6, 5, 1, 5, 3, 7, 0, 3, 4, 2, 2, 5, 7, 8, 8, 6, 3,
                                   3, 5,
                                   0, 5, 6, 2, 3, 7, 8, 8, 6, 8, 2, 3, 0, 9, 9, 3, 8, 7, 9, 0, 1, 4, 9, 4, 3, 5, 9, 1,
                                   5, 9,
                                   8, 2, 0, 2, 5, 4, 5, 7, 8, 3, 0, 5, 8, 2, 5, 8, 3, 5, 4, 9, 9, 4, 2, 5, 4, 9, 9, 8,
                                   6, 2,
                                   7, 0, 2, 6, 2, 7, 8, 7, 2, 1, 3, 5, 4, 6, 4, 8, 1, 0, 8, 8, 1, 5, 2, 1, 6, 4, 2, 6,
                                   2,
                                   8};
      utils::matches_vec expected = {
              utils::match(0, 0, 6, 6, 0.0f),
              utils::match(1, 1, 5, 5, 0.0f),
              utils::match(3, 3, 3, 3, 0.0f),
              utils::match(4, 4, 9, 9, 0.0f),
              utils::match(5, 5, 4, 4, 0.0f),
              utils::match(6, 6, 2, 2, 0.0f),
              utils::match(7, 7, 0, 0, 0.0f),
              utils::match(9, 9, 1, 1, 0.0f),
              utils::match(10, 10, 7, 7, 0.0f),
              utils::match(11, 11, 8, 8, 0.0f),
      };

      Munkres mm(12, 10);
      mm.process(costs);
      mm.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }

    }


    TEST(munkres, test_munkres_max1) {

      utils::matches_vec matches;
      utils::matches_vec expected = {
              utils::match(0, 0, 1, 1, 0.0f),
              utils::match(1, 1, 0, 0, 0.0f),
      };

      std::vector<double> costs = {1, 6, 8, 2};

      Munkres mm(2, 2, false);
      mm.process(costs);
      mm.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }

    }


    TEST(munkres, test_munkres_max2) {

      utils::matches_vec matches;
      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0f),
              utils::match(1, 1, 1, 1, 0.0f),
              utils::match(2, 2, 2, 2, 0.0f),
              utils::match(3, 3, 3, 3, 0.0f),
              utils::match(5, 5, 4, 4, 0.0f),
              utils::match(6, 6, 5, 5, 0.0f),
              utils::match(7, 7, 6, 6, 0.0f),
              utils::match(8, 8, 7, 7, 0.0f),
              utils::match(9, 9, 8, 8, 0.0f),
              utils::match(10, 10, 9, 9, 0.0f),
              utils::match(11, 11, 10, 10, 0.0f),
              utils::match(12, 12, 11, 11, 0.0f),
              utils::match(13, 13, 12, 12, 0.0f),
              utils::match(14, 14, 13, 13, 0.0f),
              utils::match(15, 15, 14, 14, 0.0f),
              utils::match(16, 16, 15, 15, 0.0f),
              utils::match(17, 17, 16, 16, 0.0f),
              utils::match(18, 18, 17, 17, 0.0f),
              utils::match(19, 19, 18, 18, 0.0f),

      };
      std::vector<double> costs = {0.6238, 0.3068, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.2614, 1.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.1436, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.3303, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.1698, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.3504, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0367, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.3314,
                                   0.0000,
                                   0.0000, 0.0000, 0.0878, 0.0000, 0.0000, 0.0830, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.5346, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0583, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0525, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.6238, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.1569, 0.0000,
                                   0.0000,
                                   0.0000, 0.6416, 0.0000, 0.0000, 0.1328, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.6594,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.6477, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.1607,
                                   0.0000, 0.0000, 0.0000, 0.1157, 0.0000, 0.0000, 0.5662, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.7237, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.7968, 0.1425, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.1090, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.1277,
                                   0.6641, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0434, 0.0000, 0.0000, 0.0850, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.6165,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.1754, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.1089, 0.0000, 0.0000, 0.7756, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
                                   0.0000,
                                   0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000};

      Munkres mm(20, 19, false);
      mm.process(costs);
      mm.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }

    }


    TEST(munkres, test_FilterMatches1) {

      utils::matches_vec matches = {
              utils::match(0, 0, 0, 0, 0.0f),
      };
      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;
      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(0, std::make_pair(0, dummy)));
      scorelist.push_back(smap);

      search::FilterMatches(matches, scorelist);

      ASSERT_EQ(matches.size(), 0);
    }


    TEST(munkres, test_FilterMatches2) {

      utils::matches_vec matches = {
              utils::match(0, 0, 0, 0, 0.0),
      };
      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;
      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(0.5, std::make_pair(0, dummy)));
      scorelist.push_back(smap);

      search::FilterMatches(matches, scorelist);

      ASSERT_EQ(matches.size(), 1);
    }


    TEST(munkres, test_FilterMatches3) {

      utils::matches_vec matches = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 1, 0.0),
              utils::match(2, 2, 2, 2, 0.0),
              utils::match(3, 3, 3, 3, 0.0),
      };
      utils::matches_vec expected = {
              utils::match(2, 2, 2, 2, 0.0),
      };

      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;
      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(0.71, std::make_pair(0, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(0, std::make_pair(1, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(0.72, std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(0.6, std::make_pair(0, dummy)));
      scorelist.push_back(smap);

      search::FilterMatches(matches, scorelist, 0.71);

      ASSERT_EQ(matches.size(), 1);
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }
    }


    TEST(dynamic, test_dynamic1) {

      utils::matches_vec matches;
      utils::matches_vec expected = {
              utils::match(0, 0, 1, 1, 0.0),
              utils::match(2, 2, 2, 2, 0.0),
      };

      std::vector<utils::scoremap> scorelist;
      std::vector<int> dummy;

      utils::scoremap smap;
      smap.insert(utils::scoremap::value_type(.1, std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(.9, std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(.5, std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(.2, std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(.1, std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(.3, std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      smap.clear();
      smap.insert(utils::scoremap::value_type(.3, std::make_pair(0, dummy)));
      smap.insert(utils::scoremap::value_type(.4, std::make_pair(1, dummy)));
      smap.insert(utils::scoremap::value_type(.8, std::make_pair(2, dummy)));
      scorelist.push_back(smap);

      Dynamic dd(3, 3);
      dd.process(scorelist);
      dd.extract_matches(matches);

      ASSERT_EQ(matches.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matches.begin(), matches.end(), e) != matches.end());
      }
    }

} // namespace
