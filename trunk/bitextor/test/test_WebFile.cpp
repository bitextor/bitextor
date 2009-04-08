#include "test_WebFile.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestWebFile );

void TestWebFile::setUp()
{
	setlocale(LC_CTYPE, "");
	GlobalParams::LoadGlobalParams("./config.xml");
	wf.Initialize("./test_files/WebFile4.html");
}

void TestWebFile::tearDown()
{
	GlobalParams::Clear();
}

void TestWebFile::testGetLang()
{
	CPPUNIT_ASSERT_EQUAL(Config::toString(wf.GetLang()),Config::toString(L"en"));
}

void TestWebFile::testGetPath()
{
	CPPUNIT_ASSERT_EQUAL(wf.GetPath(),(string)"./test_files/WebFile4.html");
}

void TestWebFile::testGetFileType()
{
	CPPUNIT_ASSERT_EQUAL(wf.GetFileType(),(string)"html");
}

void TestWebFile::testIsInitialized()
{
	CPPUNIT_ASSERT_EQUAL(true,wf.IsInitialized());
}

void TestWebFile::testGetNumbersVector()
{
	vector<int>* v=wf.GetNumbersVector();
	CPPUNIT_ASSERT_EQUAL((size_t)21,v->size());
	CPPUNIT_ASSERT_EQUAL(1,v->at(0));
	CPPUNIT_ASSERT_EQUAL(0,v->at(1));
	CPPUNIT_ASSERT_EQUAL(2,v->at(2));
	CPPUNIT_ASSERT_EQUAL(0,v->at(3));
	CPPUNIT_ASSERT_EQUAL(1,v->at(4));
	CPPUNIT_ASSERT_EQUAL(22,v->at(5));
	CPPUNIT_ASSERT_EQUAL(2008,v->at(6));
	CPPUNIT_ASSERT_EQUAL(0,v->at(7));
	CPPUNIT_ASSERT_EQUAL(99,v->at(8));
	CPPUNIT_ASSERT_EQUAL(0,v->at(9));
	CPPUNIT_ASSERT_EQUAL(0,v->at(10));
	CPPUNIT_ASSERT_EQUAL(2,v->at(11));
	CPPUNIT_ASSERT_EQUAL(6,v->at(12));
	CPPUNIT_ASSERT_EQUAL(2,v->at(13));
	CPPUNIT_ASSERT_EQUAL(2,v->at(14));
	CPPUNIT_ASSERT_EQUAL(6,v->at(15));
	CPPUNIT_ASSERT_EQUAL(0,v->at(16));
	CPPUNIT_ASSERT_EQUAL(2,v->at(17));
	CPPUNIT_ASSERT_EQUAL(0,v->at(18));
	CPPUNIT_ASSERT_EQUAL(1,v->at(19));
	CPPUNIT_ASSERT_EQUAL(2008,v->at(20));
}

void TestWebFile::testGetTextSize()
{
	CPPUNIT_ASSERT_EQUAL((unsigned int)1629,wf.GetTextSize());
}
