#include "test_Heuristics.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestHeuristics );

void TestHeuristics::setUp()
{
	setlocale(LC_CTYPE, "");
	wf1.Initialize("./test_files/WebFile1.html");
	wf2.Initialize("./test_files/WebFile2.html");
	wf3.Initialize("./test_files/WebFile3.html");
	wf4.Initialize("./test_files/WebFile4.html");
	wf5.Initialize("./test_files/WebFile7.php");
	GlobalParams::LoadGlobalParams("./config.xml");
}

void TestHeuristics::tearDown()
{
	GlobalParams::Clear();
}

void TestHeuristics::testHaveTheSameExtension()
{
	CPPUNIT_ASSERT_EQUAL(true,Heuristics::HaveTheSameExtension(&wf2,&wf4));
	CPPUNIT_ASSERT_EQUAL(false,Heuristics::HaveTheSameExtension(&wf4,&wf5));
}

void TestHeuristics::testHaveAcceptableSizeDifference()
{
	CPPUNIT_ASSERT_EQUAL(true,Heuristics::HaveAcceptableSizeDifference(&wf4,&wf5));
	CPPUNIT_ASSERT_EQUAL(false,Heuristics::HaveAcceptableSizeDifference(&wf4,&wf2));
}

void TestHeuristics::testHaveAcceptableEditDistance()
{
	CPPUNIT_ASSERT_EQUAL(true,Heuristics::HaveAcceptableEditDistance(&wf4,&wf5));
	CPPUNIT_ASSERT_EQUAL(false,Heuristics::HaveAcceptableEditDistance(&wf4,&wf2));
}

void TestHeuristics::testCost()
{
	CPPUNIT_ASSERT_EQUAL((double)1,Heuristics::Cost(SUBST,5,2));
	CPPUNIT_ASSERT_EQUAL((double)0,Heuristics::Cost(SUBST,5,4));
	CPPUNIT_ASSERT_EQUAL((double)1,Heuristics::Cost(DELETE,5,3));
	CPPUNIT_ASSERT_EQUAL((double)1,Heuristics::Cost(INSERT,5,3));
}

void TestHeuristics::testDistanceInNumericFingerprint()
{
	CPPUNIT_ASSERT_EQUAL(true,Heuristics::DistanceInNumericFingerprint(wf4,wf4));
	CPPUNIT_ASSERT_EQUAL(false,Heuristics::DistanceInNumericFingerprint(wf4,wf5));
	CPPUNIT_ASSERT_EQUAL(false,Heuristics::DistanceInNumericFingerprint(wf4,wf2));
}

void TestHeuristics::testCostNumbers()
{
	CPPUNIT_ASSERT_EQUAL((double)0,Heuristics::CostNumbers(SUBST,5,5));
	CPPUNIT_ASSERT_EQUAL((double)1,Heuristics::CostNumbers(SUBST,5,4));
	CPPUNIT_ASSERT_EQUAL((double)1,Heuristics::CostNumbers(DELETE,5,3));
	CPPUNIT_ASSERT_EQUAL((double)1,Heuristics::CostNumbers(INSERT,5,3));
}

void TestHeuristics::testNearTotalTextSize()
{
	CPPUNIT_ASSERT_EQUAL(true,Heuristics::NearTotalTextSize(wf4,wf5));
	CPPUNIT_ASSERT_EQUAL(false,Heuristics::NearTotalTextSize(wf4,wf2));
}
