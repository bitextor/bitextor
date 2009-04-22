#include "test_Bitext.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestBitext );

void TestBitext::setUp()
{
	setlocale(LC_CTYPE, "");
	wf1.Initialize("./test_files/WebFile1.html");
	wf2.Initialize("./test_files/WebFile2.html");
	wf3.Initialize("./test_files/WebFile3.html");
	wf4.Initialize("./test_files/WebFile4.html");
	wf5.Initialize("./test_files/WebFile5.php");
	wf6.Initialize("./test_files/WebFile6.html");
	wf7.Initialize("./test_files/WebFile7.html");
	GlobalParams::LoadGlobalParams("./config.xml");
}

void TestBitext::tearDown()
{
	GlobalParams::Clear();
}

void TestBitext::testInitialize()
{
	Bitext b;
	CPPUNIT_ASSERT_EQUAL(false,b.Initialize(&wf2,&wf4));
	CPPUNIT_ASSERT_EQUAL(true,b.Initialize(&wf6,&wf7));
}

void TestBitext::testGetFirstWebFile()
{
	Bitext b;
	b.Initialize(&wf4,&wf5);
	CPPUNIT_ASSERT_THROW(b.GetFirstWebFile(),char* const);
	b.Initialize(&wf6,&wf7);
	CPPUNIT_ASSERT_EQUAL(&wf6, b.GetFirstWebFile());
}

void TestBitext::testGetSecondWebFile()
{
	Bitext b;
	b.Initialize(&wf4,&wf5);
	CPPUNIT_ASSERT_THROW(b.GetSecondWebFile(),char* const);
	b.Initialize(&wf6,&wf7);
	CPPUNIT_ASSERT_EQUAL(&wf7, b.GetSecondWebFile());
}

void TestBitext::testGetSameExtension()
{
	Bitext b;
	b.Initialize(&wf4,&wf5);
	CPPUNIT_ASSERT_THROW(b.GetSameExtension(),char* const);
	b.Initialize(&wf6,&wf7);
	CPPUNIT_ASSERT(b.GetSameExtension());
}

void TestBitext::testGetSizeDistance()
{
	ifstream f;
	Bitext b;
	b.Initialize(&wf4,&wf5);
	int wf1_size, wf2_size;
	unsigned int init, end;

	CPPUNIT_ASSERT_THROW(b.GetSizeDistance(),char* const);

	f.open(wf6.GetPath().c_str());
	init=f.tellg();
	f.seekg(0, ios::end);
	end=f.tellg();
	f.close();
	wf1_size=end-init;

	f.open(wf7.GetPath().c_str());
	init=f.tellg();
	f.seekg(0, ios::end);
	end=f.tellg();
	f.close();
	wf2_size=end-init;

	b.Initialize(&wf6,&wf7);
	CPPUNIT_ASSERT_EQUAL(((double)abs(wf1_size-wf2_size)/wf2_size)*100,b.GetSizeDistance());
}

void TestBitext::testGetEditDistance()
{
	Bitext b;
	b.Initialize(&wf4,&wf5);
	CPPUNIT_ASSERT_THROW(b.GetEditDistance(),char* const);
	b.Initialize(&wf6,&wf7);
	CPPUNIT_ASSERT_EQUAL((double)0,b.GetEditDistance());
}

void TestBitext::testIsBetterThan()
{
	Bitext b1, b2, b3;
	b1.Initialize(&wf4,&wf5);
	b2.Initialize(&wf4,&wf6);
	b3.Initialize(&wf6,&wf7);
	CPPUNIT_ASSERT(!b1.IsInitialized());
	CPPUNIT_ASSERT(b2.IsInitialized());
	CPPUNIT_ASSERT(b3.IsInitialized());
	CPPUNIT_ASSERT_THROW(b1.isBetterThan(b2),char* const);
	CPPUNIT_ASSERT_EQUAL(false,b2.isBetterThan(b1));
	CPPUNIT_ASSERT_EQUAL(true,b2.isBetterThan(b3));
	CPPUNIT_ASSERT_EQUAL(false,b3.isBetterThan(b2));
}
