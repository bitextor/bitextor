#include "test_text.h"

//CPPUNIT_TEST_SUITE_REGISTRATION( TestText );

void 
TestText::setUp(){}

void 
TestText::tearDown(){}

void TestText::testConstructor()
{
	Text t1(L"");
	Text t2(L"aaa");
	
	CPPUNIT_ASSERT_EQUAL((string)"",Config::toString(t1.getString()));
	CPPUNIT_ASSERT_EQUAL((string)"aaa",Config::toString(t2.getString()));
}

void TestText::testsOverloadedConstructor()
{
	Text t1(L"aaa");
	Text t2(t1);

	CPPUNIT_ASSERT_EQUAL(Config::toString(t2.getString()),Config::toString(t1.getString()));
	CPPUNIT_ASSERT_EQUAL(t2.getLength(),t1.getLength());
	CPPUNIT_ASSERT(&t2!=&t1);
}

void TestText::testOperatorPlus()
{
	Text t1(L"aaa"), t2(L"bbb"), t3(L""), t4(L""), t5(L"");
	t5=t1+t2;
	CPPUNIT_ASSERT_EQUAL(Config::toString(t5.getString()),(string)"aaabbb");
	t5=t3+t4;
	CPPUNIT_ASSERT_EQUAL(Config::toString(t5.getString()),(string)"");
}

void TestText::testOperatorEqual()
{
	Text t1(L"aaa"), t2(L"bbb"), t3(L""), t4(L""), t5(t1+t2), t6(t3+t4+t1);
	CPPUNIT_ASSERT_EQUAL(Config::toString(t5.getString()),(string)"aaabbb");
	t5=t3+t4;
	CPPUNIT_ASSERT_EQUAL(Config::toString(t6.getString()),(string)"aaa");
}

void TestText::testOperatorClaudators()
{
	Text t1(L"aaa"), t2(L"bbb"), t3(L""), t4(L""), t5(L"");
	t5=t1;
	CPPUNIT_ASSERT_EQUAL(Config::toString(t5.getString()),(string)"aaa");
	t5=t3+t4;
	CPPUNIT_ASSERT_EQUAL(Config::toString(t5.getString()),(string)"");
}
