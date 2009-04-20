#ifndef TEST_GLOBALPARAMS_H_
#define TEST_GLOBALPARAMS_H_

#include <GlobalParams.h>
#include <TestFixture.h>
#include <extensions/HelperMacros.h>

using namespace std;

class TestGlobalParams : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestGlobalParams);
	CPPUNIT_TEST(setUp);
	CPPUNIT_TEST(testGetMaxEditDistancePercentual);
	CPPUNIT_TEST(testGetMaxEditDistanceAbsolute);
	CPPUNIT_TEST(testGetTextDistancePercentDifferenciator);
	CPPUNIT_TEST(testSetTextDistancePercentDifferenciator);
	CPPUNIT_TEST(testGetDirectoryDepthDistance);
	CPPUNIT_TEST(testSetDirectoryDepthDistance);
	CPPUNIT_TEST(testSetFileSizeDifferencePercent);
	CPPUNIT_TEST(testGetTextCatConfigFile);
	CPPUNIT_TEST(testSetTextCatConfigFile);
	CPPUNIT_TEST(testGetMaxDownloadedSize);
	CPPUNIT_TEST(testGetDownloadPath);
	CPPUNIT_TEST(testGetGuessLanguage);
	CPPUNIT_TEST(testSetGuessLanguage);
	CPPUNIT_TEST(testGetFileSizeDiferencePercent);
	CPPUNIT_TEST(testAddFileSizeDiferencePercent);
	CPPUNIT_TEST(testGetMaxNumericFingerprintDistance);
	CPPUNIT_TEST(testAllBitextInAFile);
	CPPUNIT_TEST(testGetMinArraySize);
	CPPUNIT_TEST(testGetCreateAllCandidates);
	CPPUNIT_TEST(testIsVerbose);
	CPPUNIT_TEST(testSetVerbose);
	CPPUNIT_TEST(testGetGenerateAmbiguousBitexts);
	CPPUNIT_TEST(tearDown);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void testGetMaxEditDistancePercentual();
	void testGetMaxEditDistanceAbsolute();
	void testGetTextDistancePercentDifferenciator();
	void testSetDirectoryDepthDistance();
	void testSetTextDistancePercentDifferenciator();
	void testGetDirectoryDepthDistance();
	void testGetFileSizeDifferencePercent();
	void testSetFileSizeDifferencePercent();
	void testSetTextCatConfigFile();
	void testGetTextCatConfigFile();
	void testGetMaxDownloadedSize();
	void testGetDownloadPath();
	void testSetGuessLanguage();
	void testGetGuessLanguage();
	void testAddFileSizeDiferencePercent();
	void testGetFileSizeDiferencePercent();
	void testGetMaxTotalTextLengthDiff();
	void testGetMaxNumericFingerprintDistance();
	void testAllBitextInAFile();
	void testGetMinArraySize();
	void testGetCreateAllCandidates();
	void testIsVerbose();
	void testSetVerbose();
	void testGetGenerateAmbiguousBitexts();
	void tearDown();
};

#endif /*TEST_GLOBALPARAMS_H_*/
