
![Banner](img/banner.png?raw=true)
=====

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

`bitextor` is a tool for automatically harvesting bitexts from multilingual websites. To run it, it is necessary to provide:
1. The source where the parallel data will be searched: a web site (the URL of the web site), a list of web sites, an [LETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor#LETT) file, or the path to a directory containing a crawled website.
2. The two languages on which the user is interested: language IDs must be provided following the ISO [639-1](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
3. A source of bilingual information between these two languages: either a bilingual lexicon (such as those available at the [bitextor-data repository](https://github.com/bitextor/bitextor-data/tree/master/dics)) or a machine translation system, depending on the document-alignment strategy choosen

The tool works following a sequence of steps (scripts sorted by default use):

1. Downloads a website by using the tool creepy or httrack: see module `bitextor-crawl` and `bitextor-downloadweb` (optional step);
2. The files in the website are analysed, cleaned and standardised: see module `bitextor-crawl2ett` or `bitextor-webdir2ett` (optional as related with previous step);
3. The language of every web page is detected: see module `bitextor-ett2lett` (optional, in case you give `bitextor` a [LETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor#LETT) file as input);
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


## Dependencies

Apart from downloading all submodules of this repository (you can do it with `git clone --recurse-submodules https://github.com/bitextor/bitextor.git` if you are cloning this repo from scratch or in case you are downloading a tarball just do `git submodule update --init --recursive`), there are some external tools that need to be in the path before installing the project. **autotools** and **pkg-config** are necessary for building and installing the project. Tools from **JDK** as javac and jar are needed for building Java dependences, and the virtual machine of Java is needed for running them. Also **maven** for some dependencies. In addition, a c++ compiler is required for compiling as **g++**, and **cmake** and **libboost-all-dev** for `clustercat` and `mgiza` projects. Optionally, **httrack** can be used for crawling if specified through arguments and found in binary path.

For these system libraries and tools we used apt because we are in a Debian-like environment and tested them in Ubuntu 14.04, 16.04 and 18.04. In case you have another package manager, just run the equivalent installation with it, but we cannot ensure that the versions and interfaces match the Debian ones, or even exist. In case of any problem, just search how to install those packages, including Java JDK (Oracle or OpenJDK), pip3 (with get_pip.py) and libmagic with Python interface (https://github.com/threatstack/libmagic/tree/master/) in your distribution or from source code.

`user@pc:~$ sudo apt install cmake g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libboost-all-dev maven libbz2-dev liblzma-dev zlib1g-dev libffi-dev`

Most of the scripts in bitextor are written in Python 3 syntax. Because of this, it is necessary to also install Python >= 3. All these tools are available in most Unix-based operating systems repositories.

Some external Python libraries should also be installed before starting the installation of bitextor:

- **python-Levenshtein**: Python library for computing the Levenshtein edit-distance.
- **LangID.py**: Python library for plain text language detection.
- **regex**: Python package for regular expressions in Python.
- **NLTK**: Python package with natural language processing utilities.
- **numpy**: Python package for scientific computing with Python.
- **keras**: Python package for implementing neural networks for deep learning.
- **h5py**: Pythonic interface to the HDF5 binary data format.
- **python-magic**: Python interface for the magic library, used to detect files' format (install from apt or source code in https://github.com/threatstack/libmagic/tree/master/python, not from pip: it has a different interface). It can be installed with `pip`
using `pip3 install -e git://github.com/mammadori/magic-python.git#egg=Magic_file_extensions` (useful for installing in a virtualenv).
- **iso-639**: Python package to convert between language names and ISO-639 codes

Also, Bitextor modules have alternative implementations from other pipelines, which have these dependencies:
- **bs4**: BeautifulSoup4 is a Python package for HTML/XML processing and cleaning
- **html2txt**: text extractor from HTML, created by Aaron Swartz
- **cld2**: Chromium language detector, by Google. Install through pip3 package `cld2-cffi`

We expect this project to be compatible with latest version of all previous dependencies. So that, the easiest way to install these Python libraries is using the tool pip3 (https://pypi.python.org/pypi/pip). To install or upgrade all the basic libraries at the same time, you can simply run:

`user@pc:~$ sudo pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py ftfy warc3-wet`

To ensure you are using the minimum required versions of the needed libraries, if you didn't run the previous command, you should run:

`user@pc:~$ sudo pip3 install -r requirements.txt`

If you are using Ubuntu 14.04 install tensorflow 1.4.1 because of [breaking change with glibc version in version tensorflow 1.5.0](https://github.com/tensorflow/tensorflow/releases/tag/v1.5.0-rc0), also do that if you are using an [AMD CPU and you are having errors when importing](https://github.com/tensorflow/tensorflow/issues/17411):

`sudo pip3 install tensorflow==1.4.1`

Most of these pip3 packages are also available in the repositories of many Unix-based systems, but usually `pip` ones are up to date.


### Optional dependences

In case you want to use HTTrack instead of integrated Creepy crawler just:
`sudo apt install httrack`

In addition to the Python libraries, the tool Apertium (http://www.apertium.org/) may be necessary if you plan to use lemmatisation with bitextor crawl websites containing texts in highly inflective languages. If you need this functionally, just use the option `--with-apertium` when running the `autogen.sh` configuration script at the install step.

For optional Bicleaner submodule just run `sudo pip3 install -r bicleaner/requirements.txt`

For optional Zipporah submodule, SRILM `ngram` (http://www.speech.sri.com/projects/srilm/download.html) binary is needed in PATH, and:

`sudo pip3 install --upgrade matplotlib sklearn numpy && sudo apt install python3-tk`

For optional document aligner from Paracrawl team read the [document-aligner/README.md](https://github.com/paracrawl/document-aligner/blob/master/README.md) file to install all dependencies in Python 3.

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

where LOCALDIR can be any directory where you have writing permission, such as `~/local`.

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
- boilerpipe: a tool for cleaning HTML files to remove useless information such as menus, banners, etc. (<https://code.google.com/p/boilerpipe/>) (using Maven)


## Run

There are many ways to call bitextor, but here we will explain the three most used. Two of them include the first step (downloading the websites) and are:
```
bitextor [OPTIONS] -v LEXICON -u  URL LANG1 LANG2
bitextor [OPTIONS] -v LEXICON -U FILE LANG1 LANG2
```
where, *LANG1* and *LANG2* are the two-character lang codes following the ISO 639-1, and *LEXICON* is a bilingual lexicon bewteen languages LANG1 and LANG2.
With option *-u* bitextor downloads the URL specified; option *-U* allows to use a tab-separated file containing, in each line, a URL to be crawled and the [ETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor#ETT) file where the crawled data will be stored.

It is also possible to re-process a previously crawled web site by using option *-e* to specify an [ETT](https://github.com/bitextor/bitextor/wiki/Intermediate-formats-used-in-Bitextor#ETT) (this process starts in the second step described in the previous section): 
```
bitextor [OPTIONS] -v LEXICON -e ETT LANG1 LANG2
```
Options *-u* and *-e* can be combined to specify the file where the documents downloaded from the URL will be stored for future processing.

See more useful options, entry points and stop points of the whole pipeline using -h or --help command. Options can be provided either when calling bitextor or by definning them in a configuration file as in the following example:
```
bitextor --config-file CONFIGFILE -u URL LANG1 LANG2
```
A sample configuration file (`baseline.conf`) can be found in this repository. This sample configuration file assumes that the path to the bilingual lexicons is in `bitextor-dictionaries` relative path folder from your running path. Change it if it is necessary.

Some dictionaries are provided in [bitextor-data](https://github.com/bitextor/bitextor-data) repository, but customised dictionaries can easily be built from parallel corpora as explained in the next section. It is also possible to use other lexicons already available, such as those in [OPUS](http://opus.nlpl.eu/), as long as their format is compatible with the one defined here [in this page of the wiki](https://github.com/bitextor/bitextor/wiki/Build-bilingual-lexicons-from-parallel-corpora).

Test your installation and parameters with this [sample of running bitext](https://github.com/bitextor/bitextor/wiki/Sample-of-a-bitext).
