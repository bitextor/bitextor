package com.bitextor.pipedtika;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

/**
 * Unit test for simple PipedTika.
 */
public class PipedTikaTest 
    extends TestCase
{
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public PipedTikaTest( String testName )
    {
        super( testName );
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite()
    {
        return new TestSuite( PipedTikaTest.class );
    }

    /**
     * Rigourous Test :-)
     */
    public void testPipedTika()
    {
        assertTrue( true );
    }
}
