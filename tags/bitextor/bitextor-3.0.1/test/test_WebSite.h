#ifndef TEST_WEBSITE_H_
#define TEST_WEBSITE_H_

#include <WebSite.h>
#include <TestFixture.h>
#include <extensions/HelperMacros.h>

using namespace std;

class TestWebSite : public CPPUNIT_NS::TestFixture
{
	WebSite *ws;
	CPPUNIT_TEST_SUITE(TestWebSite);
	CPPUNIT_TEST(setUp);
	CPPUNIT_TEST(testGetBasePath);
	CPPUNIT_TEST(testGetFileName);
	CPPUNIT_TEST(tearDown);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
	void testGetBasePath();
	void testGetFileName();
	
};

#endif /*TEST_WEBSITE_H_*/
