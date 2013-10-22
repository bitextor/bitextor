#include "test_WebSite.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestWebSite );

void TestWebSite::setUp()
{
	setlocale(LC_CTYPE, "");
	GlobalParams::LoadGlobalParams("./config.xml");
	ws=new WebSite("./test_files");
}

void TestWebSite::tearDown()
{
	//free(ws);
	GlobalParams::Clear();
}

void TestWebSite::testGetBasePath()
{
	CPPUNIT_ASSERT_EQUAL(ws->GetBasePath(),(string)"./test_files");
}

void TestWebSite::testGetFileName()
{
	CPPUNIT_ASSERT_EQUAL(ws->GetFileName("/home/prova/de/nom/de/fitxer/nom_de_fitxer"),(string)"nom_de_fitxer");
	CPPUNIT_ASSERT_EQUAL(ws->GetFileName("nom_de_fitxer"),(string)"nom_de_fitxer");
}
