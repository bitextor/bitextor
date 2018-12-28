/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package pipedboilerpipe;

import java.io.*;
import de.l3s.boilerpipe.BoilerpipeProcessingException;
import de.l3s.boilerpipe.document.TextDocument;
import de.l3s.boilerpipe.extractors.ArticleExtractor;
import de.l3s.boilerpipe.sax.BoilerpipeSAXInput;
import de.l3s.boilerpipe.sax.HTMLHighlighter;
import java.io.StringReader;
import java.util.Scanner;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import java.util.Base64;
import java.io.UnsupportedEncodingException;
/**
 * Java tool that reads HTML documents from STDIN (one per line) and applies
 * the ArticleExtractor in BoilerPipe to clean it. Java tool that reads HTML
 * documents from STDIN (one per line) and applies the ArticleExtractor in
 * BoilerPipe to clean it
 */
public class PipedBoilerpipe {
    /**
     * Java tool that reads HTML documents from STDIN (one per line) and applies
     * the ArticleExtractor in BoilerPipe to clean it
     * @param args the command line arguments
     */
    public static void main(String[] args) throws Exception
    {
        String rootDir = args[0];
        System.err.println("rootDir=" + rootDir);

        String mimeFile = rootDir + "/mime.txt";
        BufferedReader mimeReader = new BufferedReader(new FileReader(mimeFile));

        String pageFile = rootDir + "/raw-html/page";
        BufferedReader pageReader = new BufferedReader(new FileReader(pageFile));

        String pageLine;
        int lineNum = 0;
        while ((pageLine = pageReader.readLine()) != null) {
            String[] fields = new String[4];

            assert(pageLine != null);
            String[] toksPage = pageLine.split("\t");
            assert(toksPage.length == 2);
            fields[2] = toksPage[0];

            String mimeLine = mimeReader.readLine();
            assert(mimeLine != null);
            String[] toksMime = mimeLine.split("\t");
            assert(toksMime.length == 2);
            fields[0] = toksMime[0];
            fields[1] = toksMime[1];

            //System.err.println(fields.length);

            String file = rootDir + "/cleaned-html/" + lineNum + ".txt";
            BufferedReader fileReader = new BufferedReader(new FileReader(file));

            String lineFile = "";
            String st;
            while ((st = fileReader.readLine()) != null) {
                //System.err.println(st);
                lineFile += st + "\n";
            }
            String line = lineFile;

            //Processing XHTML
            StringReader reader = new StringReader(line);
            TextDocument source = new BoilerpipeSAXInput(new InputSource(reader)).getTextDocument();
            //Processing XHTML to remove boilerplates
            ArticleExtractor extractor=ArticleExtractor.INSTANCE;
            extractor.process(source);
            //Producing clean XHTML
            HTMLHighlighter h=HTMLHighlighter.newExtractingInstance();

            byte[] bytes = h.process(source, line).getBytes("UTF-8");

            String outFile = rootDir + "/deboiled/" + lineNum;
            OutputStream outStream = new FileOutputStream(outFile);
            outStream.write(bytes);
            outStream.close();

            ++lineNum;
        }
    }
}
