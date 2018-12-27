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
        System.err.println(rootDir);

        Scanner stdin = new Scanner(System.in);
        int lineNum = 0;
        while(stdin.hasNextLine())
        {
            String[] fields=stdin.nextLine().split("\t");
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

            //Reading a line
            //String line = new String(Base64.getDecoder().decode(fields[3]), "UTF-8");
            //Processing XHTML
            StringReader reader = new StringReader(line);
            TextDocument source = new BoilerpipeSAXInput(new InputSource(reader)).getTextDocument();
            //Processing XHTML to remove boilerplates
            ArticleExtractor extractor=ArticleExtractor.INSTANCE;
            extractor.process(source);
            //Producing clean XHTML
            HTMLHighlighter h=HTMLHighlighter.newExtractingInstance();

            byte[] bytes = h.process(source, line).getBytes("UTF-8");
            String encoded = Base64.getEncoder().encodeToString(bytes);
            fields[3]=encoded;
            StringBuilder sb=new StringBuilder();
            for(String f: fields){
                sb.append(f);
                sb.append("\t");
            }
            sb.deleteCharAt(sb.length()-1);
            System.out.println(sb.toString());


            ++lineNum;
        }
    }
}
