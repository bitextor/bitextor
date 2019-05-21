
#include "gtest/gtest.h"
#include "../src/scorer.h"


using namespace scorer;


namespace {

    TEST(scorer, test_ApplyNormalizeRules_normalize1_rules) {

      const static rule_pair rule1[] = {normalize1_rules[0]};
      std::string res1 = ApplyNormalizeRules("<skipped> <skipped> <skipped>", rule1);
      ASSERT_EQ(res1, "  ");

      const static rule_pair rule2[] = {normalize1_rules[1]};
      std::string res2 = ApplyNormalizeRules("-\n-\n\n-\n-", rule2);
      ASSERT_EQ(res2, "\n-");

      const static rule_pair rule3[] = {normalize1_rules[2]};
      std::string res3 = ApplyNormalizeRules("-\n-\n\n-\n-", rule3);
      ASSERT_EQ(res3, "- -  - -");

      std::string res_all = ApplyNormalizeRules("\n<skipped>-\n-\n\n-<skipped>\n\n<skipped>\n-", normalize1_rules);
      ASSERT_EQ(res_all, "    -");

    }


    TEST(scorer, test_ApplyNormalizeRules_normalize2_rules) {

      std::string res1 = ApplyNormalizeRules("0&amp;1&gt;&gt;2&lt;3&quot;4", normalize2_rules);
      ASSERT_EQ(res1, "0&1>>2<3\"4");

    }


    TEST(scorer, test_Tokenize) {
      std::vector<std::string> token_vec;

      std::string text0 = "more than three million albanians 123-123 living outside albania, the majority of @ them (about 2.5 million) in kosovo, but $$ also in macedonia and montenegro.";
      std::vector<std::string> expected0 = {"more", "than", "three", "million", "albanians", "123", "-", "123",
                                            "living",
                                            "outside", "albania", ",", "the", "majority", "of", "@", "them", "(",
                                            "about",
                                            "2.5", "million", ")", "in", "kosovo", ",", "but", "$", "$", "also", "in",
                                            "macedonia", "and", "montenegro", "."};
      scorer::Tokenize(token_vec, text0);

      for (size_t i = 0; i < expected0.size(); ++i) {
        if (i >= token_vec.size()) FAIL();
        ASSERT_EQ(expected0.at(i), token_vec.at(i));
      }

      std::string text1 = "#more than 3-000-000 albanians living outside albania, the majority of them (~2.5 million) @ kosovo, but also in macedonia and montenegro.";
      std::vector<std::string> expected1 = {"#", "more", "than", "3", "-", "000", "-", "000", "albanians", "living",
                                            "outside", "albania", ",", "the", "majority", "of", "them", "(", "~", "2.5",
                                            "million", ")", "@", "kosovo", ",", "but", "also", "in", "macedonia", "and",
                                            "montenegro", "."};
      scorer::Tokenize(token_vec, text1);
      for (size_t i = 0; i < expected1.size(); ++i) {
        if (i >= token_vec.size()) FAIL();
        ASSERT_EQ(expected1.at(i), token_vec.at(i));
      }

      std::string text2 = "BOEING 777-200 - 280 SEATS.Characteristics Length in meters: 63.70 Wingspan in meters: 60.90 Cruising speed: Mach .84 Cruising altitude: 10 700 m / 35 000 ft";
      std::vector<std::string> expected2 = {"BOEING", "777", "-", "200", "-", "280", "SEATS", ".", "Characteristics",
                                            "Length", "in", "meters", ":", "63.70", "Wingspan", "in", "meters", ":",
                                            "60.90", "Cruising", "speed", ":", "Mach", ".", "84", "Cruising",
                                            "altitude",
                                            ":", "10", "700", "m", "/", "35", "000", "ft"};
      scorer::Tokenize(token_vec, text2);
      for (size_t i = 0; i < expected2.size(); ++i) {
        if (i >= token_vec.size()) FAIL();
        ASSERT_EQ(expected2.at(i), token_vec.at(i));
      }

      SUCCEED();
    }


    TEST(scorer, test_normalize) {
      std::vector<std::string> token_vec;

      std::string s1 = "In the hard-to-reach north (northern Albanian Alps), where foreign occupants whether Ottoman, Austro-Hungarian, Italian or German special effort had ever exert any control, there were some relics of the old tribal structures and traditions long fort and make some still felt today.";
      std::vector<std::string> expected_vec1 = {"in", "the", "hard-to-reach", "north", "(", "northern", "albanian",
                                                "alps",
                                                ")", ",", "where", "foreign", "occupants", "whether", "ottoman", ",",
                                                "austro-hungarian", ",", "italian", "or", "german", "special", "effort",
                                                "had", "ever", "exert", "any", "control", ",", "there", "were", "some",
                                                "relics", "of", "the", "old", "tribal", "structures", "and",
                                                "traditions",
                                                "long", "fort", "and", "make", "some", "still", "felt", "today", "."};
      scorer::normalize(token_vec, s1, "western");
      for (size_t i = 0; i < expected_vec1.size(); ++i) {
        if (i >= token_vec.size()) FAIL();
        ASSERT_EQ(expected_vec1.at(i), token_vec.at(i));
      }


      std::string s2 = "During the Turkish rule were about 70% of the Albanians to Islam about 20% were Orthodox and Catholics almost 10%.";
      std::vector<std::string> expected_vec2 = {"during", "the", "turkish", "rule", "were", "about", "70", "%", "of",
                                                "the",
                                                "albanians", "to", "islam", "about", "20", "%", "were", "orthodox",
                                                "and",
                                                "catholics", "almost", "10", "%", "."};
      scorer::normalize(token_vec, s2, "western");
      for (size_t i = 0; i < expected_vec2.size(); ++i) {
        if (i >= token_vec.size()) FAIL();
        ASSERT_EQ(expected_vec2.at(i), token_vec.at(i));
      }

      std::string s3 = "You are here: Home » Country and People » The Albanian People deutsch|english|shqip Navigation Home About us Distribution Business Partner Contact Picture Gallery Albatec Country and People Map of Albania The Albanian People About Albania Picture Gallery Albania Imprint The Albanian People The Albanians consider themselves as descendants of the Illyrians, and hence as autochthonous Balkan nation.";
      std::vector<std::string> expected_vec3 = {"you", "are", "here", ":", "home", "»", "country", "and", "people", "»",
                                                "the", "albanian", "people", "deutsch", "|", "english", "|", "shqip",
                                                "navigation", "home", "about", "us", "distribution", "business",
                                                "partner",
                                                "contact", "picture", "gallery", "albatec", "country", "and", "people",
                                                "map", "of", "albania", "the", "albanian", "people", "about", "albania",
                                                "picture", "gallery", "albania", "imprint", "the", "albanian", "people",
                                                "the", "albanians", "consider", "themselves", "as", "descendants", "of",
                                                "the", "illyrians", ",", "and", "hence", "as", "autochthonous",
                                                "balkan",
                                                "nation", "."};
      scorer::normalize(token_vec, s3, "western");
      for (size_t i = 0; i < expected_vec3.size(); ++i) {
        if (i >= token_vec.size()) FAIL();
        ASSERT_EQ(expected_vec3.at(i), token_vec.at(i));
      }

    }

} // namespace
