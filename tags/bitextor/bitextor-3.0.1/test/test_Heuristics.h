#ifndef TEST_HEURISTICS_H_
#define TEST_HEURISTICS_H_

#include <Heuristics.h>
#include <TestFixture.h>
#include <extensions/HelperMacros.h>

using namespace std;

class TestHeuristics : public CPPUNIT_NS::TestFixture
{
	WebFile wf1, wf2, wf3, wf4, wf5, wf6, wf7;
	CPPUNIT_TEST_SUITE(TestHeuristics);
	CPPUNIT_TEST(setUp);
	CPPUNIT_TEST(testHaveTheSameExtension);
	CPPUNIT_TEST(testHaveAcceptableSizeDifference);
	CPPUNIT_TEST(testHaveAcceptableEditDistance);
	CPPUNIT_TEST(testCost);
	CPPUNIT_TEST(testDistanceInNumericFingerprint);
	CPPUNIT_TEST(testCostNumbers);
	CPPUNIT_TEST(testCostNumbers);
	CPPUNIT_TEST(testNearTotalTextSize);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
	void testHaveTheSameExtension();
	void testHaveAcceptableSizeDifference();
	void testHaveAcceptableEditDistance();
	void testCost();
	void testDistanceInNumericFingerprint();
	void testCostNumbers();
	void testNearTotalTextSize();
};

#endif /*TEST_HEURISTICS_H_*/
