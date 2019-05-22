
#include "gtest/gtest.h"
#include "../src/align.h"
#include "../src/utils/common.h"

#include <iostream>
#include <string>
#include <vector>
#include <boost/make_unique.hpp>


namespace {

    TEST(align, test_align) {

      std::vector<std::string> text2_doc = {
              "Albatec | The Albanian People Skip to the navigation .",
              "Skip to the content .",
              "With friends and guests to share them if necessary their last piece of bread.",
              "In the hard-to-reach north (northern Albanian Alps), where foreign occupants whether Ottoman, Austro-Hungarian, Italian or German special effort had ever exert any control, there were some relics of the old tribal structures and traditions long fort and make some still felt today.",
              "There was also the blood revenge longer than elsewhere.",
              "During the Turkish rule were about 70% of the Albanians to Islam about 20% were Orthodox and Catholics almost 10%.",
              "Since 1967 Constitution by any religion prohibited, churches and mosques were warehouses or sports facilities pesos, and some served as museums.",
              "This is now everything has been undone.",
              "What the ethnic composition concerned, it is about 97% of the Albanian population, with the remaining 3% are composed of Greeks, Macedonians, Montenegrinen and Roma.",
              "The country, which before the war, only one million inhabitants, has a high birthrate increase over three and a half million people and one of 38 to 70 years increased life expectancy.",
      };

      std::vector<std::string> text1translated_doc = {
              "Albatec | Die Albaner Skip to the navigation .",
              "Skip to the content .",
              "on the other hand , are the Albanians in contrast to many of the &apos; benefits &apos; of the industrial society pampered Europeans , of course , very open @-@ minded and warm .",
              "with friends and guests share them if need be her last bit bread .",
              "Im schwer zugänglichen Norden (Nordalbanische Alpen), wo fremde Okkupanten ob Osmanische, Österreich-Ungarische, Italienische oder Deutsche besondere Mühe hatten, überhaupt irgendeine Kontrolle auszuüben, bestanden manche Relikte der alten Stammesstrukturen und Bräuche lange fort und machen sich teilweise noch heute bemerkbar.",
              "this was also the vendetta longer than elsewhere .",
              "during the Türkenherrschaft came about 70 % of the Albanians to Islam , on 20 % were Orthodox and less than 10 % Catholics .",
              "since 1967 was constitution prohibited by any religion , churches and mosques were stockrooms or sports , turned into some served as museums .",
              "this is now everything undone .",
              "what the ethnic Zusammenetzung are concerned , roughly 97 % of the population Albanians , the rest of the 3 % are made up of Greeks , Macedonians , Montenegrinen and Roma together .",
      };

      std::vector<float> expected_scores = {0.623753, 1, 0.0877724, 0.534555, 0.0583084, 0.623753, 0.64162, 0.65938,
                                            0.647696, 0.566179};
      std::vector<int> expected_refs = {0, 1, 1, 0, 5, 8, 2, 3, 4, 5, 8, 6, 7, 8, 5};
      std::vector<int> expected_correct_unigram = {7, 4, 5, 4, 7, 11, 10, 8, 7, 19, 9, 20, 6, 24, 8};
      std::vector<int> expected_correct_bigram = {5, 2, 4, 2, 2, 1, 6, 1, 5, 13, 2, 12, 4, 15, 2};

      std::vector<utils::scoremap> scorelist;
      align::EvalSents(scorelist, text1translated_doc, text2_doc, 2, 2);

      int pos = 0;
      for (size_t s = 0; s < scorelist.size(); ++s) {
        EXPECT_NEAR(scorelist.at(s).rbegin()->first, expected_scores.at(s), 0.01);

        utils::scoremap::reverse_iterator rev_it = scorelist.at(s).rbegin();
        while (rev_it != scorelist.at(s).rend()) {
          ASSERT_EQ(rev_it->second.first, expected_refs.at(pos));
          ASSERT_EQ(rev_it->second.second.at(0), expected_correct_unigram.at(pos));
          ASSERT_EQ(rev_it->second.second.at(1), expected_correct_bigram.at(pos));
          ++pos;
          ++rev_it;
        }

      }

    }


    TEST(align, test_align_emptyscorelist) {

      std::vector<std::string> text2_doc = {
              "Albatec | The Albanian People Skip to the navigation .",
              "Skip to the content .",
      };

      std::vector<std::string> text1translated_doc = {
              "during the Türkenherrschaft came about 70 % of the Albanians to Islam , on 20 % were Orthodox and less than 10 % Catholics .",
              "since 1967 was constitution prohibited by any religion , churches and mosques were stockrooms or sports , turned into some served as museums .",
              "this is now everything undone .",
      };

      std::vector<utils::scoremap> scorelist;
      align::EvalSents(scorelist, text1translated_doc, text2_doc, 2, 2);
      ASSERT_EQ(scorelist.size(), 3);

    }


    TEST(align, test_GapFiller1) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 1, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
              utils::match(3, 3, 2, 2, 0.0),
              utils::match(4, 4, 4, 4, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 1, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
              utils::match(3, 3, 2, 2, 0.0),
              utils::match(4, 4, 4, 4, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock on this blog and the clock on my laptop are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
              "They arrived early and got really good seats.",
              "How was math test?",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop are 1 hour different from each other.",
              "They got there early, and they got really good seats.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
              "How was the math test?",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller2) {
      utils::matches_vec matched = {};

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
      };

      std::vector<std::string> english = {
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), 0);

    }


    TEST(align, test_GapFiller_race) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(2, 2, 1, 1, 0.0),
              utils::match(3, 3, 3, 3, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 1, 0, 0, 0.0),
              utils::match(2, 2, 1, 2, 0.0),
              utils::match(3, 3, 3, 3, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until", // merge-pregap
              "we realize that we are all the same.", // merge-pregap
              "I am happy to take your donation; any amount will be greatly appreciated.",
              "Each amount is highly valued and helps to pay for tools and assets",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "I am glad to take your donation;", // merge-postgap
              "Every amount is much appreciated ", // merge-postgap
              "and helps pay for tools and assets", // no merge (taken)
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_pregap0) {
      utils::matches_vec matched = {
              utils::match(1, 1, 0, 0, 0.0),
              utils::match(2, 2, 1, 1, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 1, 0, 0, 0.0),
              utils::match(2, 2, 1, 1, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until",
              "we realize that we are all the same.",
              "The clock on this blog and the clock on my laptop are 1 hour apart.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_pregap1) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 2, 2, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 2, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock on this blog and the clock on my laptop are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop",
              "are 1 hour apart.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 2, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_pregap2) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(3, 3, 1, 1, 0.0),
              utils::match(4, 4, 2, 2, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 3, 1, 1, 0.0),
              utils::match(4, 4, 2, 2, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock on this",
              "blog and the clock",
              "on my laptop are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop are 1 hour different from each other.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_pregap3) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
              utils::match(3, 3, 4, 4, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 2, 1, 3, 0.0),
              utils::match(3, 3, 4, 4, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock",
              "on this blog and the watch on my computer are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog",
              "and the clock on my laptop are",
              "1 hour different from each other.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: matched) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_postgap0) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(2, 2, 1, 1, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 1, 0, 0, 0.0),
              utils::match(2, 2, 1, 1, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until",
              "we realize that we are all the same.",
              "The clock on this blog and the clock on my laptop are 1 hour apart.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_postgap1) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 1, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 2, 0.0),
              utils::match(2, 2, 3, 3, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock on this blog and the clock on my laptop are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop",
              "are 1 hour apart.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 2, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_postgap2) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 1, 0.0),
              utils::match(4, 4, 2, 2, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 3, 1, 1, 0.0),
              utils::match(4, 4, 2, 2, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock on this",
              "blog and the clock",
              "on my laptop are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog and the clock on my laptop are 1 hour different from each other.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_GapFiller_postgap3) {
      utils::matches_vec matched = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 1, 1, 1, 0.0),
              utils::match(3, 3, 4, 4, 0.0),
      };

      utils::matches_vec expected = {
              utils::match(0, 0, 0, 0, 0.0),
              utils::match(1, 2, 1, 3, 0.0),
              utils::match(3, 3, 4, 4, 0.0),
      };

      std::vector<std::string> translated = {
              "Let everyone be unique until we realize that we are all the same.",
              "The clock",
              "on this blog and the watch on my computer are 1 hour apart.",
              "I am glad to take your donation; every amount is much appreciated.",
      };

      std::vector<std::string> english = {
              "Lets all be unique together until we realise we are all the same.",
              "The clock within this blog",
              "and the clock on my laptop are",
              "1 hour different from each other.",
              "I am happy to take your donation; any amount will be greatly appreciated.",
      };

      align::GapFiller(matched, translated, english, 3, 0.0);

      ASSERT_EQ(matched.size(), expected.size());
      for (auto e: expected) {
        ASSERT_TRUE(std::find(matched.begin(), matched.end(), e) != matched.end());
      }

    }


    TEST(align, test_ProduceMergedSentences) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
              "We have a lot of rain in June.",
              "We need to rent a room for our party.",
              "Abstraction is often one floor above you.",
      };

      std::vector<std::string> expected_text = {
              "it won't suit me. ",
              "it won't suit me. We have a lot of rain in June. ",
              "it won't suit me. We have a lot of rain in June. We need to rent a room for our party. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(1, 1),
              std::make_pair(1, 2),
              std::make_pair(1, 3),
      };


      align::ProduceMergedSentences(merged_text, merged_pos, text_doc, 1, 4, 3);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_ProduceMergedSentences_rev) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
              "We have a lot of rain in June.",
              "We need to rent a room for our party.",
              "Abstraction is often one floor above you.",
      };

      std::vector<std::string> expected_text = {
              "We need to rent a room for our party. ",
              "We have a lot of rain in June. We need to rent a room for our party. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(3, 3),
              std::make_pair(2, 3),
      };


      align::ProduceMergedSentences(merged_text, merged_pos, text_doc, 1, 3, 2, true);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_PreGapMergedSentences1) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
              "We have a lot of rain in June.",
              "We need to rent a room for our party.",
              "Abstraction is often one floor above you.",
      };

      std::vector<std::string> expected_text = {
              "We need to rent a room for our party. ",
              "We have a lot of rain in June. We need to rent a room for our party. ",
              "it won't suit me. We have a lot of rain in June. We need to rent a room for our party. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(3, 3),
              std::make_pair(2, 3),
              std::make_pair(1, 3),
      };

      std::unique_ptr<int[]> matches_arr = boost::make_unique<int[]>(text_doc.size());
      std::fill(matches_arr.get(), matches_arr.get() + text_doc.size(), -1);

      align::PreGapMergedSentences(merged_text, merged_pos, text_doc, matches_arr, 3, 3);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_PreGapMergedSentences2) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
              "We have a lot of rain in June.",
              "We need to rent a room for our party.",
              "Abstraction is often one floor above you.",
      };

      std::vector<std::string> expected_text = {
              "We need to rent a room for our party. ",
              "We have a lot of rain in June. We need to rent a room for our party. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(3, 3),
              std::make_pair(2, 3),
      };

      std::unique_ptr<int[]> matches_arr = boost::make_unique<int[]>(text_doc.size());
      std::fill(matches_arr.get(), matches_arr.get() + text_doc.size(), -1);

      matches_arr[1] = 0;
      align::PreGapMergedSentences(merged_text, merged_pos, text_doc, matches_arr, 3, 3);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_PreGapMergedSentences3) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
      };

      std::vector<std::string> expected_text = {
              "Two seats were vacant. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(0, 0),
      };

      std::unique_ptr<int[]> matches_arr = boost::make_unique<int[]>(text_doc.size());
      std::fill(matches_arr.get(), matches_arr.get() + text_doc.size(), -1);

      align::PreGapMergedSentences(merged_text, merged_pos, text_doc, matches_arr, 0, 5);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_PostGapMergedSentences1) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
              "We have a lot of rain in June.",
              "We need to rent a room for our party.",
              "Abstraction is often one floor above you.",
      };

      std::vector<std::string> expected_text = {
              "it won't suit me. ",
              "it won't suit me. We have a lot of rain in June. ",
              "it won't suit me. We have a lot of rain in June. We need to rent a room for our party. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(1, 1),
              std::make_pair(1, 2),
              std::make_pair(1, 3),
      };

      std::unique_ptr<int[]> matches_arr = boost::make_unique<int[]>(text_doc.size());
      std::fill(matches_arr.get(), matches_arr.get() + text_doc.size(), -1);

      align::PostGapMergedSentences(merged_text, merged_pos, text_doc, matches_arr, text_doc.size(), 1, 3);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_PostGapMergedSentences2) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
              "We have a lot of rain in June.",
              "We need to rent a room for our party.",
              "Abstraction is often one floor above you.",
      };

      std::vector<std::string> expected_text = {
              "it won't suit me. ",
              "it won't suit me. We have a lot of rain in June. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(1, 1),
              std::make_pair(1, 2),
      };

      std::unique_ptr<int[]> matches_arr = boost::make_unique<int[]>(text_doc.size());
      std::fill(matches_arr.get(), matches_arr.get() + text_doc.size(), -1);

      matches_arr[3] = 0;
      align::PostGapMergedSentences(merged_text, merged_pos, text_doc, matches_arr, text_doc.size(), 1, 3);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_PostGapMergedSentences3) {
      std::vector<std::string> merged_text;
      utils::vec_pair merged_pos;

      std::vector<std::string> text_doc = {
              "Two seats were vacant.",
              "it won't suit me.",
      };

      std::vector<std::string> expected_text = {
              "it won't suit me. ",
      };

      utils::vec_pair expected_pos = {
              std::make_pair(1, 1),
      };

      std::unique_ptr<int[]> matches_arr = boost::make_unique<int[]>(text_doc.size());
      std::fill(matches_arr.get(), matches_arr.get() + text_doc.size(), -1);

      align::PostGapMergedSentences(merged_text, merged_pos, text_doc, matches_arr, text_doc.size(), 1, 5);

      ASSERT_EQ(merged_text.size(), expected_text.size());
      for (size_t i = 0; i < expected_text.size(); ++i) {
        ASSERT_EQ(expected_text[i], merged_text[i]);
      }

      ASSERT_EQ(merged_pos.size(), expected_pos.size());
      for (size_t i = 0; i < expected_pos.size(); ++i) {
        ASSERT_EQ(expected_pos[i], merged_pos[i]);
      }

    }


    TEST(align, test_FillMatches1) {
      size_t arr1_size = 3;
      size_t arr2_size = 4;
      std::unique_ptr<int[]> arr1 = boost::make_unique<int[]>(arr1_size);
      std::fill(arr1.get(), arr1.get() + arr1_size, -1);

      std::unique_ptr<int[]> arr2 = boost::make_unique<int[]>(arr2_size);
      std::fill(arr2.get(), arr2.get() + arr2_size, -1);

      int arr1_expected[] = {1, 1, 1};
      int arr2_expected[] = {-1, 0, 0, -1};

      align::FillMatches(arr1, arr2, utils::match(0, 2, 1, 2, 0.0));

      for (size_t i = 0; i < arr1_size; ++i) {
        ASSERT_EQ(arr1_expected[i], arr1[i]);
      }

      for (size_t i = 0; i < arr2_size; ++i) {
        ASSERT_EQ(arr2_expected[i], arr2[i]);
      }
    }


    TEST(align, test_FillMatches2) {
      size_t arr1_size = 5;
      size_t arr2_size = 5;
      std::unique_ptr<int[]> arr1 = boost::make_unique<int[]>(arr1_size);
      std::fill(arr1.get(), arr1.get() + arr1_size, -1);

      std::unique_ptr<int[]> arr2 = boost::make_unique<int[]>(arr2_size);
      std::fill(arr2.get(), arr2.get() + arr2_size, -1);

      int arr1_expected[] = {-1, 2, 2, 2, 2};
      int arr2_expected[] = {-1, -1, 1, 2, -1};

      align::FillMatches(arr1, arr2, utils::match(2, 2, 3, 3, 0.0));
      align::FillMatches(arr1, arr2, utils::match(3, 3, 2, 2, 0.0));
      align::FillMatches(arr1, arr2, utils::match(1, 4, 2, 2, 0.0));

      for (size_t i = 0; i < arr1_size; ++i) {
        ASSERT_EQ(arr1_expected[i], arr1[i]);
      }

      for (size_t i = 0; i < arr2_size; ++i) {
        ASSERT_EQ(arr2_expected[i], arr2[i]);
      }
    }

} // namespace
