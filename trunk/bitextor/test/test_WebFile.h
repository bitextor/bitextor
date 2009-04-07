#ifndef TEST_FRAGMENT_H_
#define TEST_FRAGMENT_H_

#include <WebFile.h>
#include <TestFixture.h>
#include <extensions/HelperMacros.h>

using namespace std;

class TestWebFile : public CPPUNIT_NS::TestFixture
{
	WebFile wf;
	CPPUNIT_TEST_SUITE(TestWebFile);
	CPPUNIT_TEST(setUp);
	CPPUNIT_TEST(testGetLang);
	CPPUNIT_TEST(testGetPath);
	CPPUNIT_TEST(testGetFileType);
	CPPUNIT_TEST(testIsInitialized);
	CPPUNIT_TEST(testGetNumbersVector);
	CPPUNIT_TEST(testGetTextSize);
	CPPUNIT_TEST(tearDown);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
	void testGetLang();
	void testGetPath();
	void testGetFileType();
	void testIsInitialized();
	void testGetNumbersVector();
	void testGetTextSize();
};

#endif /*TEST_FRAGMENT_H_*/
