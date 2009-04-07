#ifndef TEST_FRAGMENT_H_
#define TEST_FRAGMENT_H_

#include <WebSite.h>
#include <TestFixture.h>
#include <extensions/HelperMacros.h>

using namespace std;

class TestWebSite : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestWebSite);
	CPPUNIT_TEST(setUp);
	CPPUNIT_TEST(tearDown);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
};

#endif /*TEST_FRAGMENT_H_*/
