#ifndef TEST_TEXT_H_
#define TEST_TEXT_H_

#include "../tagaligner/text.h"
#include <TestFixture.h>
#include <extensions/HelperMacros.h>


using namespace std;

class TestText : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestText);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testsOverloadedConstructor);
	CPPUNIT_TEST(testOperatorPlus);
	CPPUNIT_TEST(testOperatorEqual);
	CPPUNIT_TEST(testOperatorClaudators);
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	void testConstructor();
	void testsOverloadedConstructor();
	void testOperatorPlus();
	void testOperatorEqual();
	void testOperatorClaudators();
};

#endif /*TEST_TEXT_H_*/
