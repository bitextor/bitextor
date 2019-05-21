
#include "gtest/gtest.h"
#include "../src/utils/common.h"

#include <boost/functional.hpp>


namespace {

    TEST(utils, test_common_PieceToString) {

      std::string s1("Hello");
      StringPiece sp1(s1);
      ASSERT_EQ(s1, sp1.data());

      std::string s2 = utils::PieceToString(sp1);
      ASSERT_EQ(s1, s2);

      s1 = "World";
      ASSERT_EQ(sp1.data(), s1);
      ASSERT_EQ(s2, "Hello");
    }


    TEST(utils, test_common_SplitStringPiece) {

      std::string s1("This is a text with many single spaces and   a  few     gaps   . ");
      StringPiece sp1(s1);

      std::vector<StringPiece> sp_vec1;
      utils::SplitStringPiece(sp_vec1, sp1, ' ');
      ASSERT_EQ(sp_vec1.size(), 23);
      std::vector<std::string> expected_vec1 = {"This", "is", "a", "text", "with", "many", "single", "spaces", "and",
                                                "",
                                                "", "a", "", "few", "", "", "", "", "gaps", "", "", ".", ""};
      for (size_t i = 0; i < expected_vec1.size(); ++i) {
        if (i >= sp_vec1.size()) FAIL();
        ASSERT_EQ(expected_vec1.at(i), sp_vec1.at(i));
      }

      std::vector<StringPiece> sp_vec2;
      utils::SplitStringPiece(sp_vec2, sp1, ' ', 10);
      ASSERT_EQ(sp_vec1.size(), 23);
      ASSERT_EQ(sp_vec2.size(), 20);
      std::vector<std::string> expected_vec2 = {"text", "with", "many", "single", "spaces", "and", "", "", "a", "",
                                                "few",
                                                "", "", "", "", "gaps", "", "", ".", ""};
      for (size_t i = 0; i < expected_vec2.size(); ++i) {
        if (i >= sp_vec2.size()) FAIL();
        ASSERT_EQ(expected_vec2.at(i), sp_vec2.at(i));
      }

      std::vector<StringPiece> sp_vec3;
      utils::SplitStringPiece(sp_vec3, sp1, ' ', 10, 7);
      ASSERT_EQ(sp_vec1.size(), 23);
      ASSERT_EQ(sp_vec2.size(), 20);
      ASSERT_EQ(sp_vec3.size(), 8);
      std::vector<std::string> expected_vec3 = {"text", "with", "many", "single", "spaces", "and", "",
                                                " a  few     gaps   . "};
      for (size_t i = 0; i < expected_vec3.size(); ++i) {
        if (i >= sp_vec3.size()) FAIL();
        ASSERT_EQ(expected_vec3.at(i), sp_vec3.at(i));
      }


    }


    TEST(utils, test_common_SplitStringPiece_func) {

      // testing the same behaviour with isspace
      std::string s1("This is a text with many single spaces and   a  few     gaps   . ");
      StringPiece sp1(s1);

      std::vector<StringPiece> sp_vec1;
      utils::SplitStringPiece(sp_vec1, sp1, &std::isspace);
      ASSERT_EQ(sp_vec1.size(), 23);
      std::vector<std::string> expected_vec1 = {"This", "is", "a", "text", "with", "many", "single", "spaces", "and",
                                                "",
                                                "", "a", "", "few", "", "", "", "", "gaps", "", "", ".", ""};
      for (size_t i = 0; i < expected_vec1.size(); ++i) {
        if (i >= sp_vec1.size()) FAIL();
        ASSERT_EQ(expected_vec1.at(i), sp_vec1.at(i));
      }


      std::string s2(
              "This\tis a text\t\twith spaces,tabulators\t\t\t, \n new lines\n,\vvertical tab\v, \ffeed\f,and carriage return\r\r");
      StringPiece sp2(s2);

      // using a single space
      std::vector<StringPiece> sp_vec2;
      utils::SplitStringPiece(sp_vec2, sp2, ' ');
      ASSERT_EQ(sp_vec2.size(), 11);
      std::vector<std::string> expected_vec2 = {"This\tis", "a", "text\t\twith", "spaces,tabulators\t\t\t,", "\n",
                                                "new",
                                                "lines\n,\vvertical", "tab\x0b,", "\ffeed\f,and", "carriage",
                                                "return\r\r"};
      for (size_t i = 0; i < expected_vec2.size(); ++i) {
        if (i >= sp_vec2.size()) FAIL();
        ASSERT_EQ(expected_vec2.at(i), sp_vec2.at(i));
      }

      // using isspace function
      std::vector<StringPiece> sp_vec3;
      utils::SplitStringPiece(sp_vec3, s2, &std::isspace);
      ASSERT_EQ(sp_vec3.size(), 25);
      std::vector<std::string> expected_vec3 = {"This", "is", "a", "text", "", "with", "spaces,tabulators", "", "", ",",
                                                "",
                                                "", "new", "lines", ",", "vertical", "tab", ",", "", "feed", ",and",
                                                "carriage", "return", "", ""};
      for (size_t i = 0; i < expected_vec3.size(); ++i) {
        if (i >= sp_vec3.size()) FAIL();
        ASSERT_EQ(expected_vec3.at(i), sp_vec3.at(i));
      }

    }

} // namespace
