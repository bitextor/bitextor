#include "test_WebFile.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestWebSite );

void TestWebSite::setUp()
{
	setlocale(LC_CTYPE, "");
	GlobalParams::LoadGlobalParams("./config.xml");
}

void TestWebSite::tearDown()
{
	GlobalParams::Clear();
}
