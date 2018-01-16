
package pipedtika;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.FileNotFoundException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import org.apache.commons.codec.Charsets;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.sax.SAXTransformerFactory;
import javax.xml.transform.sax.TransformerHandler;
import javax.xml.transform.stream.StreamResult;
import org.apache.tika.parser.AutoDetectParser;
import org.apache.tika.Tika;
import org.apache.tika.exception.TikaException;
import org.apache.tika.metadata.Metadata;
import org.apache.tika.parser.ParseContext;
import org.apache.tika.sax.BodyContentHandler;
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;
import org.apache.commons.codec.binary.Base64;

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
            OutputStream output, String method, boolean prettyPrint)
            throws TransformerConfigurationException {
        SAXTransformerFactory factory = (SAXTransformerFactory)
                SAXTransformerFactory.newInstance();
        TransformerHandler handler = factory.newTransformerHandler();
        handler.getTransformer().setOutputProperty(OutputKeys.METHOD, method);
        handler.getTransformer().setOutputProperty(OutputKeys.INDENT, prettyPrint ? "yes" : "no");
        handler.getTransformer().setOutputProperty(OutputKeys.ENCODING, "UTF-8");
        handler.setResult(new StreamResult(output));
        return handler;
    }

    /**
     * Main class that takes the list of files from the output and process it.
     * The input contains additional information: it is tab-sepparated and the name of the file is in field 3
     * @param args the command line arguments
     */
    public static void main(String[] args) throws FileNotFoundException, IOException, TransformerConfigurationException, SAXException {
        BufferedReader br=new BufferedReader(new InputStreamReader(System.in));
        String line;
        //Tika t=new Tika();
        AutoDetectParser parser = new AutoDetectParser();
        OutputStream os;
        //Reading a file path from stdin each time
        while((line=br.readLine())!=null){
            String[] fields=line.split("\t");
            if(fields.length>=2){
                //Option for setting if the output is XML or TXT
                boolean returnxml=true;
                if(args.length>0){
                    if(args[0].equals("-x") || args[0].equals("--xml")){
                        returnxml=true;
                    }
                    else if(args[0].equals("-t") || args[0].equals("--text")){
                        returnxml=false;
                    }
                }
                //Building the new handler
                ContentHandler handler;
                os=new ByteArrayOutputStream();
                InputStream is;
                String filecontent, content, inputencoding;
//                try{
//                    if(fields[1].contains("="))
//                        inputencoding=fields[1].split("=")[1];
//                    else
//                        inputencoding=fields[1];
//
//                    filecontent=new String(Base64.decodeBase64(fields[3]), inputencoding);
//                    content=filecontent.replaceAll("\\s+", " ").replaceAll("\n\\s*", "\n").trim();
//                    is = new ByteArrayInputStream(content.getBytes(inputencoding));
//                } catch(java.io.UnsupportedEncodingException ex){
                    filecontent=new String(Base64.decodeBase64(fields[3]), "utf-8");
                    content=filecontent.replaceAll("\\s+", " ").replaceAll("\n\\s*", "\n").replaceAll("<meta charset=([^<]*)>", "" ).replaceAll("<meta http-equiv=\"content-type\" content=([^<]*)>", "" ).trim();
                    is = new ByteArrayInputStream(content.getBytes("utf-8"));
//                }

                if(returnxml)
                    handler=getTransformerHandler(os, "xml", true);
                else
                    handler=new BodyContentHandler((new OutputStreamWriter(os, "UTF-8")));

                //Parsing
                try{
                    Metadata metadata = new Metadata();
                    metadata.set(Metadata.CONTENT_ENCODING, "UTF-8");
                    parser.parse(is, handler, metadata, new ParseContext());
                    if(returnxml)
                        System.out.println(line+"\t"+new String(os.toString().replace("\n", " ").replaceAll("\\s+", " ").getBytes(), "UTF-8"));
                    else
                        System.out.println(line+"\t"+new String(Base64.encodeBase64(os.toString().getBytes()), "UTF-8"));
                } catch(TikaException ex){
                    System.err.println("Error while processing file");
                }
            }
        }
    }    
}
