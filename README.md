
![Banner](img/banner.png?raw=true)
=====

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

`bitextor` is a tool to automatically harvest bitexts from multilingual websites. To run it, it is necessary to provide:
1. The source where the parallel data will be searched: one or more [website hostnames](https://en.wikipedia.org/wiki/URL)
2. The two languages on which the user is interested: language IDs must be provided following the [ISO 639-1](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
3. A source of bilingual information between these two languages: either a bilingual lexicon (such as those available at the [bitextor-data repository](https://github.com/bitextor/bitextor-data/tree/master/dics)), a machine translation (MT) system, or a parallel corpus to be used to produce either a lexicon or an MT system (depending on the document-alignment strategy choosen)

## Dependencies

Apart from downloading all submodules of this repository (you can do it with `git clone --recurse-submodules https://github.com/bitextor/bitextor.git` if you are cloning this repo from scratch or in case you are downloading a tarball just do `git submodule update --init --recursive`), there are some external tools that need to be in the path before installing the project. **autotools** and **pkg-config** are necessary for building and installing the project. Tools from **JDK** are needed to run Java dependences. In addition, a c++ compiler is required for compiling dependences. **libboost-all-dev** dependence is need to compile `clustercat` and `mgiza` projects. Optionally, **httrack** can be used for crawling if it is installed.

If you are using an apt-like package manager you can run the following command line to install all these dependences:

`user@pc:~$ sudo apt install cmake g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libboost-all-dev maven libbz2-dev liblzma-dev zlib1g-dev libffi-dev`

Most of the scripts in bitextor are written in Python 3. Because of this, it is necessary to also install Python >= 3. All these tools are available in most Unix-based operating systems repositories.

Some additional Python libraries are required. They can be installed automatically with the tool pip by runing:

`user@pc:~$ sudo pip3 install -r requirements.txt`

The web crawler HTTrack can be used in Bitextor. To do so, first install it by running the command:

`sudo apt install httrack`

This dependency is not mandatory as a second parallel data crawler is provided in Bitextor.

## Install

To install bitextor you will first need to run the script `configure` (if you are downloading the code directly from the repository you will need to run the script `autogen.sh`, which will identify the location of the external tools used. Then the code will be compiled and installed by means of the command 'make':

```
user@pc:~$ ./autogen.sh
user@pc:~$ make
```

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

## Pipeline description

Bitextor is a pipeline that runs a collection of scripts to produce a parallel corpus from a collection of multilingual websites. The pipeline is divided in five stages:
1. **Crawling**: documents are downloaded from the specified websites
2. **Pre-processing**: downloaded documents are normalized, boilerplates are removed, plain text is extracted, and language is identified
3. **Document alignment**: parallel documents are identified. Two strategies are implemented for this stage:
  1. one using bilingual lexica and a collection of features extracted from HTML; a linear regressor combines these resources to produce a score in [0,1], and
  2. another using machine translation and a [TF/IDF ](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) strategy to score document pairs
4. **Segment alignment**: each pair of documents is processed to identify parallel segments. Again, two strategies are implemented:
  1. one using the tool [Hunalign](http://mokk.bme.hu/resources/hunalign/), and
  2. another using [Bleualign](https://github.com/rsennrich/Bleualign), that can only be used if the MT-based-document-alignment strategy is used (machine translations are used for both methods)
5. **Post-processing**: final steps that allow to clean the parallel corpus obtained using the tool [bicleaner](https://github.com/bitextor/bicleaner), deduplicates translation units, and computes additional quialtity metrics

The following diagram shows the structure of the pipeline and the different scripts that are used in each stage:

![Banner](img/bitextor7.png?raw=true)
