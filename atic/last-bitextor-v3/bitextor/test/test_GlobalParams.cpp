#include "test_GlobalParams.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestGlobalParams );

void TestGlobalParams::setUp()
{
	GlobalParams::LoadGlobalParams("./config.xml");
}

void TestGlobalParams::testGetMaxEditDistancePercentual()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetMaxEditDistancePercentual(),(double)20);
}

void TestGlobalParams::testGetMaxEditDistanceAbsolute()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetMaxEditDistanceAbsolute(),(double)12);
}

void TestGlobalParams::testGetTextDistancePercentDifferenciator()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetTextDistancePercentDifferenciator(),(double)50);
}

void TestGlobalParams::testSetTextDistancePercentDifferenciator()
{
	GlobalParams::SetTextDistancePercentDifferenciator(1);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetTextDistancePercentDifferenciator(),(double)1);
}

void TestGlobalParams::testGetDirectoryDepthDistance()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetDirectoryDepthDistance(),1);
}

void TestGlobalParams::testSetDirectoryDepthDistance()
{
	GlobalParams::SetDirectoryDepthDistance(10);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetDirectoryDepthDistance(),10);
}

void TestGlobalParams::testGetFileSizeDifferencePercent()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDifferencePercent(),(double)30);
}

void TestGlobalParams::testSetFileSizeDifferencePercent()
{
	GlobalParams::SetFileSizeDifferencePercent(10);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDifferencePercent(),(double)10);
}

void TestGlobalParams::testGetTextCatConfigFile()
{
	CPPUNIT_ASSERT_EQUAL(Config::toString(GlobalParams::GetTextCatConfigFile()), Config::toString(L"/tmp/textcat_conf.txt"));
}

void TestGlobalParams::testSetTextCatConfigFile()
{
	GlobalParams::SetTextCatConfigFile(L".");
	CPPUNIT_ASSERT_EQUAL(Config::toString(GlobalParams::GetTextCatConfigFile()), Config::toString(L"."));
}

void TestGlobalParams::testGetMaxDownloadedSize()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetMaxDownloadedSize(), (int)10000);
}

void TestGlobalParams::testGetDownloadPath()
{
	CPPUNIT_ASSERT_EQUAL(Config::toString(GlobalParams::GetDownloadPath()), Config::toString(L"/home/miquel/"));
}

void TestGlobalParams::testGetGuessLanguage()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetGuessLanguage(),true);
}

void TestGlobalParams::testSetGuessLanguage()
{
	GlobalParams::SetGuessLanguage(false);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetGuessLanguage(),false);
}

void TestGlobalParams::testGetFileSizeDiferencePercent()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDiferencePercent(L"es",L"fr"),(double)38);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDiferencePercent(L"es",L"en"),(double)34);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDiferencePercent(L"es",L"ca"),(double)40);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDiferencePercent(L"noexisteix",L"tampocexisteix"),(double)50);
}

void TestGlobalParams::testAddFileSizeDiferencePercent()
{
	GlobalParams::AddFileSizeDiferencePercent(L"noexisteix",L"tampocexisteix",90);
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetFileSizeDiferencePercent(L"noexisteix",L"tampocexisteix"),(double)90);
}

void TestGlobalParams::testGetMaxTotalTextLengthDiff()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetMaxTotalTextLengthDiff(),(double)10);
}

void TestGlobalParams::testGetMaxNumericFingerprintDistance()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetMaxNumericFingerprintDistance(),(int)5);
}

void TestGlobalParams::testAllBitextInAFile()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::AllBitextInAFile(),true);
}

void TestGlobalParams::testGetMinArraySize()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetMinArraySize(),-1);
}

void TestGlobalParams::testGetCreateAllCandidates()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetCreateAllCandidates(),false);
}

void TestGlobalParams::testIsVerbose()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::IsVerbose(),true);
}

void TestGlobalParams::testSetVerbose()
{
	GlobalParams::SetVerbose();
	CPPUNIT_ASSERT_EQUAL(GlobalParams::IsVerbose(),true);
}

void TestGlobalParams::testGetGenerateAmbiguousBitexts()
{
	CPPUNIT_ASSERT_EQUAL(GlobalParams::GetGenerateAmbiguousBitexts(),(double)1);
}

void TestGlobalParams::tearDown()
{
	GlobalParams::Clear();
}
