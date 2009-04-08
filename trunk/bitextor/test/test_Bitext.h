#ifndef TEST_FRAGMENT_H_
#define TEST_FRAGMENT_H_

#include <Bitext.h>
#include <TestFixture.h>
#include <extensions/HelperMacros.h>

using namespace std;

class TestBitext : public CPPUNIT_NS::TestFixture
{
	WebFile wf1, wf2, wf3, wf4, wf5, wf6, wf7;
	CPPUNIT_TEST_SUITE(TestBitext);
	CPPUNIT_TEST(setUp);
	CPPUNIT_TEST(testInitialize);
	CPPUNIT_TEST(testGetFirstWebFile);
	CPPUNIT_TEST(testGetSecondWebFile);
	CPPUNIT_TEST(testGetSameExtension);
	CPPUNIT_TEST(testGetSizeDistance);
	CPPUNIT_TEST(testIsBetterThan);
	CPPUNIT_TEST(tearDown);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void testInitialize();
	void testGetFirstWebFile();
	void testGetSecondWebFile();
	void testGetSameExtension();
	void testGetSizeDistance();
	void testGetEditDistance();
	void testIsBetterThan();
	void tearDown();
};

#endif /*TEST_FRAGMENT_H_*/
