
package pipedtika;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.sax.SAXTransformerFactory;
import javax.xml.transform.sax.TransformerHandler;
import javax.xml.transform.stream.StreamResult;
import org.apache.tika.Tika;
import org.apache.tika.exception.TikaException;
import org.apache.tika.metadata.Metadata;
import org.apache.tika.parser.ParseContext;
import org.xml.sax.SAXException;
/**
 * Class implemented to allow tika to process a list of files without having to
 * restart the virtual machine each time. This is a simple class that takes a list
 * of files from the standard input and process it to produce XHTML UTF-8 output.
 * The result of the processing is also output in a line per file, which makes 
 * it easier to process it in a pipe.
 * 
 * @author Miquel Esplà Gomis
 */
public class PipedTika {

    /**
     * Returns a transformer handler that serializes incoming SAX events
     * to XHTML or HTML (depending the given method) using the given output
     * encoding. This method was coppied from the 1.5 version of tika
     *
     * @see <a href="https://issues.apache.org/jira/browse/TIKA-277">TIKA-277</a>
     * @param output output stream
     * @param method "xml" or "html"
     * @param encoding output encoding,
     *                 or <code>null</code> for the platform default
     * @return {@link System#out} transformer handler
     * @throws TransformerConfigurationException
     *         if the transformer can not be created
     */
    private static TransformerHandler getTransformerHandler(
            OutputStream output, String method, String encoding, boolean prettyPrint)
            throws TransformerConfigurationException {
        SAXTransformerFactory factory = (SAXTransformerFactory)
                SAXTransformerFactory.newInstance();
        TransformerHandler handler = factory.newTransformerHandler();
        handler.getTransformer().setOutputProperty(OutputKeys.METHOD, method);
        handler.getTransformer().setOutputProperty(OutputKeys.INDENT, prettyPrint ? "yes" : "no");
        if (encoding != null) {
            handler.getTransformer().setOutputProperty(
                    OutputKeys.ENCODING, encoding);
        }
        handler.setResult(new StreamResult(output));
        return handler;
    }

    /**
     * Main class that takes the list of files from the output and process it.
     * The input contains additional information: it is tab-sepparated and the name of the file is in field 3
     * @param args the command line arguments
     */
    public static void main(String[] args) throws IOException, TikaException, TransformerConfigurationException, SAXException {
        BufferedReader br=new BufferedReader(new InputStreamReader(System.in));
        String line;
        Tika t=new Tika();
        OutputStream os;
        //Reading a file path from stdin each time
        while((line=br.readLine())!=null){
            String[] fields=line.split("\t");
            if(fields.length==3){
                //Reading a line
                String filename=fields[2];
                os=new ByteArrayOutputStream();
                //Building the new handler
                TransformerHandler handler=getTransformerHandler(os, "xml", "UTF-8", true);
                InputStream is=new BufferedInputStream(new FileInputStream(filename));
                //Parsing
                t.getParser().parse(is, handler, new Metadata(), new ParseContext());
                System.out.println(line+"\t"+os.toString().replace("\n", " ").replaceAll("\\s+", " "));
            }
        }
    }    
}
