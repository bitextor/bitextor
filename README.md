
![Banner](img/banner.png?raw=true)
=====

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

`bitextor` is a tool for automatically harvesting bitexts from multilingual websites. To run it, it is necessary to provide:
1. The source where the parallel data will be searched: a web site (the URL of the web site), a list of web sites, an [LETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor) file, or the path to a directory containing a crawled website.
2. The two languages on which the user is interested: language IDs must be provided following the ISO [639-1](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
3. A source of bilingual information between these two languages: either a bilingual lexicon (such as those available at the [bitextor-data repository](https://github.com/bitextor/bitextor-data/tree/master/dics)) or a machine translation system, depending on the document-alignment strategy choosen

The tool works following a sequence of steps (scripts sorted by default use):

1. Downloads a website by using the tool creepy or httrack: see module `bitextor-crawl` and `bitextor-downloadweb` (optional step);
2. The files in the website are analysed, cleaned and standardised: see module `bitextor-crawl2ett` or `bitextor-webdir2ett` or `tar2lett` (optional as related with previous step);
3. The language of every web page is detected: see module `bitextor-ett2lett` or `tar2lett` (optional, in case you give `bitextor` a [LETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor) file as input);
4. Document align:
* Bitextor document aligner
  * The HTML structure is analysed to create a representation which is used to compare the different web pages: see module `bitextor-lett2lettr`;
  * The a preliminary list of document-alignment candidates is obtained by computing bag-of-word-overlapping measures: see modules in folder `features` ;
  * The candidates are checked by using the HTML structure: see module `bitextor-distancefilter`;
  * The documents are aligned using translation dictionaries: see module `bitextor-align-documents`;
* or Paracrawl document aligner `document-aligner/doc_align`
5. A set of aligned segments is obtained from the aligned documents, using Hunalign, and then filtered: see modules `bitextor-align-segments` and `bitextor-cleantextalign`, also optionally `zipporah-classifier` and `bicleaner/bicleaner-classifier-full`
6. The aligned segments are formatted into TMX standard format: see module `bitextor-buildTMX` (optional step, otherwise output will be a tab separated file).

It is worth noting that each of these steps can be run separately.


## Dependences

Apart from downloading all submodules of this repository (you can do it with `git clone --recurse-submodules https://github.com/bitextor/bitextor.git` if you are cloning this repo from scratch or in case you are downloading a tarball just do `git submodule update --init --recursive`), there are some external tools that need to be in the path before installing the project. **autotools** and **pkg-config** are necessary for building and installing the project. Tools from **JDK** as javac and jar are needed for building Java dependences, and the virtual machine of Java is needed for running them. In addition, a c++ compiler is required for compiling as **g++**, and **cmake** and **libboost-all-dev** for `clustercat` and `mgiza` projects. Optionally, **httrack** can be used for crawling if specified through arguments and found in binary path. See Ubuntu/Debian command below (right above "Optional dependences" headline) to install all of them at once.

Most of the scripts in bitextor are written in Python. Because of this, it is necessary to also install Python 2. All these tools are available in most Unix-based operating systems repositories.

Some external Python libraries should also be installed before starting the installation of bitextor:

- **python-Levenshtein**: Python library for computing the Levenshtein edit-distance.
- **LangID.py**: Python library for plain text language detection.
- **regex**: Python package for regular expressions in Python.
- **NLTK**: Python package with natural language processing utilities.
- **numpy**: Python package for scientific computing with Python.
- **keras**: Python package for implementing neural networks for deep learning.
- **h5py**: Pythonic interface to the HDF5 binary data format.
- **python-magic**: Python interface for the magic library, used to detect files' format (install from apt or source code in https://github.com/threatstack/libmagic/tree/master/python, not from pip: it has a different interface).
- **iso-639**: Python package to convert between language names and ISO-639 codes

Also, Bitextor modules have alternative implementations from other pipelines, which have these dependencies:
- **bs4**: BeautifulSoup4 is a Python package for HTML/XML processing and cleaning
- **html2txt**: text extractor from HTML, created by Aaron Swartz
- **cld2**: Chromium language detector, by Google. Install through pip package `cld2-cffi`

We expect this project to be compatible with latest version of all previous dependencies. So that, the easiest way to install these Python libraries is using the tool pip (https://pypi.python.org/pypi/pip). To install all the basic libraries at the same time, you can simply run:

`user@pc:~$ sudo pip install python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py`

Most of these pip packages are also available in the repositories of many Unix-based systems, but usually `pip` ones are more updated.

For system libraries and tools we used apt because we are in a Debian-like environment and tested them in Ubuntu 14.04, 16.04 and 18.04. In case you have another package manager, just run the equivalent installation with it, but we cannot ensure that the versions and interfaces match the Debian ones, or even exist. In case of any problem, just search how to install those packages, including Java JDK (Oracle or OpenJDK), pip (with get_pip.py) and libmagic with Python interface (https://github.com/threatstack/libmagic/tree/master/) in your distribution or from source code.

`user@pc:~$ sudo apt install cmake g++ automake pkg-config openjdk-8-jdk python python-pip python-magic libboost-all-dev`

### Optional dependences

In case you want to use HTTrack instead of integrated Creepy crawler just:
`sudo apt install httrack`

In addition to the Python libraries, the tool Apertium (http://www.apertium.org/) may be necessary if you plan to use lemmatisation with bitextor crawl websites containing texts in highly inflective languages. If you need this functionally, just use the option `--with-apertium` when running the `autogen.sh` configuration script at the install step.

For the alternative HTTrack to LETT process script named with `--jhu-lett`, some Python 2 dependences are needed:
`sudo pip install html2text bs4`
`sudo CFLAGS="-Wno-narrowing" pip install cld2-cffi`
```bash
wget http://corpus.tools/raw-attachment/wiki/Downloads/chared-1.2.2.tar.gz
tar xzvf chared-1.2.2.tar.gz
cd chared-1.2.2/
python setup.py install
```

For optional Bicleaner submodule `python3` is needed and then just run `sudo pip3 install -r bicleaner/requirements.txt`, in case that `pip3` is pointing to Python 3 installation. Otherwise, use the pertinent binary of `pip` of Python 3.

For optional Zipporah submodule, SRILM `ngram` (http://www.speech.sri.com/projects/srilm/download.html) binary is needed in PATH, and:
`sudo pip install matplotlib sklearn numpy && sudo apt install python-tk`

For optional document aligner from JHU read the [document-aligner/README.md](https://github.com/paracrawl/document-aligner/blob/master/README.md) file to install all dependencies in Python 3.

## Install

To install bitextor you will first need to run the script 'configure', which will identify the location of the external tools used. Then the code will be compiled and installed by means of the command 'make':

```
user@pc:~$ ./autogen.sh
user@pc:~$ make
user@pc:~$ sudo make install
```

In case you do not have sudoer privileges, it is possible to install the tool locally by specifying a different installation directory when running the script 'configure':

```
user@pc:~$ ./autogen.sh --prefix=LOCALDIR
user@pc:~$ make
user@pc:~$ make install
```

where LOCALDIR can be any directory where you have writing permission, such as ~/local.

In both examples, Apertium is an optional requirement and a warning will be prompted to the user if this tool is not installed when running configure. If you want to use this tool you can run configure with the option --with-apertium, but again, it is purely optional:

```
user@pc:~$ ./autogen.sh --prefix=LOCALDIR --with-apertium
user@pc:~$ make
user@pc:~$ make install
```

Some more tools are included in the bitextor package and will be installed together with bitextor:
- hunalign: a software for sentence alignment (<http://mokk.bme.hu/resources/hunalign/>)
- mgiza: machine translation package, here used for building probabilistic bilingual dictionaries (<https://github.com/moses-smt/mgiza>)
- clustercat: Fast Word Clustering program, parallelised alternative to mkcls (<https://github.com/jonsafari/clustercat>)
- apache tika: a tool for HTML files normalisation (<http://tika.apache.org/>)
- boilerpipe: a tool for cleaning HTML files to remove useless information such as menus, banners, etc. (<https://code.google.com/p/boilerpipe/>)


## Run

There are many ways to call bitextor, but here we will explain the three most used. Two of them include the first step (downloading the websites) and are:
```
bitextor [OPTIONS] -v LEXICON -u  URL LANG1 LANG2
bitextor [OPTIONS] -v LEXICON -U FILE LANG1 LANG2
```
where, *LANG1* and *LANG2* are the two-character lang codes following the ISO 639-1, and *LEXICON* is a bilingual lexicon bewteen languages LANG1 and LANG2.
With option *-u* bitextor downloads the URL specified; option *-U* allows to use a tab-separated file containing, in each line, a URL to be crawled and the [ETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor) file where the crawled data will be stored.

It is also possible to re-process a previously crawled web site by using option *-e* to specify an [ETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor) (this process starts in the second step described in the previous section): 
```
bitextor [OPTIONS] -v LEXICON -e ETT LANG1 LANG2
```
Options *-u* and *-e* can be combined to specify the file where the documents downloaded from the URL will be stored for future processing.

See more useful options, entry points and stop points of the whole pipeline using -h or --help command. Options can be provided either when calling bitextor or by definning them in a configuration file as in the following example:
```
bitextor --config-file CONFIGFILE -u URL LANG1 LANG2
```
A sample configuration file (`baseline.conf`) can be found in this repository. This sample configuration file assumes that the path to the bilingual lexicons is `~/bitextor-dictionaries`.

Some dictionaries are provided in [bitextor-data](https://github.com/bitextor/bitextor-data) repository, but customised dictionaries can easily be built from parallel corpora as explained in the next section. It is also possible to use other lexicons already available, such as those in [OPUS](http://opus.nlpl.eu/), as long as their format is compatible with the one defined in the next section.

It is important to highlight that Bitextor 

## Build bilingual dictionaries from parallel corpora

To create a parallel corpus, it is necessary to have a bilingual dictionary containing translations of words. These dictionaries are formatted as follows:
```
LANGUAGE1_CODE	LANGUAGE2_CODE
word1_in_language1	word1_in_language2
word2_in_language1	word2_in_language2
word3_in_language1	word3_in_language2
...	...
```
For example, a valid dictionary could be:
```
en	es
car	coche
and	y
letter	carta
...	...
```
Some dictionaries are available in https://sourceforge.net/projects/bitextor/files/bitextor/bitextor-4.0/dictionaries/ . However, customised dictionaries can be automatically built from parallel corpora. This package includes the script bitextor-builddics to ease the creation of these dictionaries. The script uses the tool GIZA++ (http://code.google.com/p/giza-pp/) to build probabilistic dictionaries, which are filtered to keep only those pairs of words fitting the following two criteria:
- both words must occur at least 10 times in the corpus; and
- the harmonic mean of translating the word from lang1 to lang2 and from lang2 to lang1 must be equal or higher than 0.2.

To obtain a dictionary, it is only needed to have a parallel corpus in the following format:
 - the corpus must be composed by two files, one containing the segments in lang1, and the other containing the segments in lang2; and
 - the segments appearing in the same line in both files must be parallel.
For a pair of files FILE1 and FILE2 containing a parallel corpus, the script would be used as follows:
```
bitextor-builddics LANG1 LANG2 FILE1 FILE2 OUTPUT
```
with OUTPUT being the path to the file which will contain the resulting dictionary.


###Sample of a bitext###

A small sample can be generated by crawling, for example, Miquel Esplà-Gomis' website (one of the developers of Bitextor): [http://www.dlsi.ua.es/~mespla](http://www.dlsi.ua.es/~mespla), which contains parallel data between Catalan and English. To do so, the English—Catalan bilingual lexicon at [https://github.com/bitextor/bitextor-data/blob/master/dics/en-ca.dic](https://github.com/bitextor/bitextor-data/blob/master/dics/en-ca.dic) can be used. Then run:

~~~~~~
:::bash
 user@pc:~$ bitextor -u http://www.dlsi.ua.es/~mespla -v en-ca.dic -x en ca
~~~~~~

A small [TMX](https://en.wikipedia.org/wiki/Translation_Memory_eXchange) translation memory will be generated:

~~~~~~
:::XML
<?xml version="1.0"?>
<tmx version="1.4">
 <header
   adminlang="ca"
   srclang="en"
   o-tmf="PlainText"
   creationtool="bitextor"
   creationtoolversion="4.0"
   datatype="PlainText"
   segtype="sentence"
   creationdate="20140817T174918"
   o-encoding="utf-8">
 </header>
 <body>
   <tu tuid="1" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>Associació Europea per a la Traducció Automàtica</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Associació Europea per a la Traducció Automàtica</seg>
    </tuv>
   </tu>
   <tu tuid="2" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>Software</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Programari</seg>
    </tuv>
   </tu>
   <tu tuid="3" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>Gamblr-CAT: Software for word-level quality estimation in computer aided translation based on translation memories.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Gamblr-CAT: Programa per a l&apos;estimació de la qualitat per a eines de traucció assistida per ordinador basada en memòries de traducció.</seg>
    </tuv>
   </tu>
   <tu tuid="4" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>This software allows to obtain binary quality estimations at the level of words (also called word-keeping recommendations) for translation suggestions produced by a translation memory tool by using either statistical word alignments or external sources of bilingual information.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Aquest programa permet obtenir estimacions de la qualitat binàries a nivell de paraula per a suggeriments de traducció proporcionats per memòries de traducció fent servir alineaments de paraules o fonts d&apos;informació bilingüe extenres.</seg>
    </tuv>
   </tu>
   <tu tuid="5" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>Gamblr-MT: Software for word-level quality estimation in machine translation.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Gamblr-MT: Programa per a l&apos;estimació de la qualitat per a traducció automàtica.</seg>
    </tuv>
   </tu>
   <tu tuid="6" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>This is a collection of scripts that allow to obtain a collection of features for word-level quality estimation using external sources of bilingual information.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Es tracta d&apos;una col·lecció de scripts que permet obtenir uan col·lecció de característiques per a l&apos;estimació de la qualitat de la traducció fent servir fonts d&apos;informació bilingüe externes.</seg>
    </tuv>
   </tu>
   <tu tuid="7" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>Bitextor: Software for building parallel data from multilingual websites.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Bitextor: Programari per a la creació de corpus paral·lels a partir de llocs web multilíngües.</seg>
    </tuv>
   </tu>
   <tu tuid="8" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>This tool is able to create a parallel corpus from a multilingual website by using only a bilingual lexicon.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Aquesta eina pot crear un corpus paral·lel a partir d&apos;un lloc multilíngüe fent servir només un lexicó bilingüe.</seg>
    </tuv>
   </tu>
   <tu tuid="9" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>OmegaT-SessionLog: Plugin for tracking the actions of a translator when using the computer aided translation based on translation memories tool OmegaT. This plugin creates an XML session log containing all the actions taken by the translation using this tool.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>OmegaT-SessionLog: Complement per a enregistrar les accions que un traductor professional duu a terme quan fa servir l&apos;eina de traducció assistida per ordinador basada en memòries de traducció OmegaT. Aquest complement crea un registre de la sessió en XML que conté totes les accions realitzades pel traductor durant el procés de traducció.</seg>
    </tuv>
   </tu>
   <tu tuid="10" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>OmegaT-Marker-Plugin: Plugin for OmegaT that implements the heuristic word-level quality estimation system.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>OmegaT-Marker-Plugin: Complement per a l&apos;eina de traducció assistida per ordinador basada en memòries de traducció OmegaT que implementa un mètode heurístic per a l&apos;estimació de la qualitat a nivell de paraules.</seg>
    </tuv>
   </tu>
   <tu tuid="11" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>This plguin allows to apply some of the methods implemented in Gamblr-CAT for word-level quality estimation in translation memories.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Aquest complement permet aplicar alguns dels mètodes implementats en Gamblr-CAT per a l&apos;estimació de la qualitat de la traducció en memòries de traducció.</seg>
    </tuv>
   </tu>
   <tu tuid="12" datatype="Text">
    <tuv xml:lang="en">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/software.html</prop>
     <seg>Flyligner: Word-alignment software based on external sources of bilingual information.</seg>
    </tuv>
    <tuv xml:lang="ca">
     <prop type="source-document">http://www.dlsi.ua.es/~mespla/programari.html</prop>
     <seg>Flyligner: Eina per a l&apos;alineament de paraules basada en fonts d&apos;informació bilingüe externes.</seg>
    </tuv>
   </tu>
 </body>
</tmx>
~~~~~~
If you want to store the translation memory in a file, use the option **-O**.



