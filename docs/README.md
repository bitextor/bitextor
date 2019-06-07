
![Banner](https://raw.githubusercontent.com/bitextor/bitextor/master/img/banner.png)
=====

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)
[![Chat on Discord](https://camo.githubusercontent.com/b4175720ede4f2621aa066ffbabb70ae30044679/68747470733a2f2f696d672e736869656c64732e696f2f62616467652f636861742d446973636f72642d627269676874677265656e2e737667)](https://discord.gg/etYDaZm)

`bitextor` is a tool to automatically harvest bitexts from multilingual websites. To run it, it is necessary to provide:
1. The source where the parallel data will be searched: one or more websites (namely, bitextor needs [website hostnames](https://en.wikipedia.org/wiki/URL))
2. The two languages on which the user is interested: language IDs must be provided following the [ISO 639-1](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
3. A source of bilingual information between these two languages: either a bilingual lexicon (such as those available at the [bitextor-data repository](https://github.com/bitextor/bitextor-data/releases/tag/bitextor-v1.0)), a machine translation (MT) system, or a parallel corpus to be used to produce either a lexicon or an MT system (depending on the alignment strategy chosen, see below)

## Dependencies

Apart from downloading all submodules of this repository (which you can do with `git clone --recurse-submodules https://github.com/bitextor/bitextor.git` if you are cloning this repo from scratch or, in case you are downloading a tarball, just do `git submodule update --init --recursive`), there are some external tools that need to be in the path before installing the project. **autotools** and **pkg-config** are necessary for building and installing the project. Tools from **JDK** are needed to run Java dependencies ([Boilerpipe](https://boilerpipe-web.appspot.com/)); version 8 or later are required. In addition, a C++ compiler is required for compiling dependencies. The **libboost-all-dev** dependency is need to compile the [`clustercat`](https://github.com/jonsafari/clustercat) and [`mgiza`](https://github.com/moses-smt/mgiza) projects. Optionally, **[httrack](https://www.httrack.com/)** and `wget` can be used for crawling if it is installed.

If you are using an apt-like package manager you can run the following command line to install all these dependencies:

`sudo apt install cmake automake pkg-config python3 python3-pip libboost-all-dev openjdk-8-jdk liblzma-dev time`

Furthermore, most of the scripts in bitextor are written in Python 3. Because of this, it is necessary to install Python >= 3. All the tools explained above are available from the repositories of most Unix-like operating systems.

Some additional Python libraries are required. They can be installed automatically with the tool pip by running (use without `sudo` if you are running in a virtualenv):

```
pip3 install -r requirements.txt
pip3 install -r bicleaner/requirements.txt https://github.com/vitaka/kenlm/archive/master.zip
pip3 install -r restorative-cleaning/requirements.txt
```
(if you have issues with `datrie` in Conda, use `conda install datrie` and try again)

As we explained above, the web crawler HTTrack can be used in Bitextor. To do so, first install it by running the command:

`sudo apt install httrack`

This dependency is not mandatory as `wget` is supported and a Python parallel data crawler is provided in Bitextor ([Creepy](https://github.com/Aitjcize/creepy)).

## Submodules compilation

To compile all bitextor submodules you will first need to run the script `configure` (if you are downloading the code directly from the GitHub repository you will need to run the script `autogen.sh` instead, which will identify the location of the external tools used). Then the code will be compiled using `make`:

`./autogen.sh && make`

### Some known installation issues

In some machines equipped with an AMD CPU you may experience some troubles with tensorflow 1.8.0 (the version specified in `requirements.txt`). In case you have installed all the requirements successfully, but when running ./autoconf.sh or ./configure you get an error that says tensorflow is not installed, please, replace the current version with version 1.5:
```bash
sudo pip3 uninstall tensorflow
sudo pip3 install tensorflow==1.5.0
```

In addition, some users haver reported problems when trying to install tensorflow using `pip3` for versions of Python >= 3.7. If this is the case, you can try to install it manually or using another package management tool, or to use a lower version of Python.

Depending on the version of *libboost* that you are using, you may experience some problems when compiling some of the sub-modules included in Bitextor. If this is the case you can install version 1.66 manually by running the following commands:
```bash
sudo apt-get remove libboost-all-dev
sudo apt-get autoremove
wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
tar xvf boost_1_66_0.tar.gz
cd boost_1_66_0/
./bootstrap.sh
./b2 -j16 --layout=system install || echo FAILURE
cd ..
rm -rf boost_1_66_0*
```
## Run

To run Bitextor use the main script `bitextor.sh`. In general, this script takes two parameters:
```bash
bitextor.sh -s <CONFIGFILE> [-j <NUMJOBS>]
```
where
* `<CONFIGFILE>` is a [YAML](https://en.wikipedia.org/wiki/YAML) configuration file containing the list of parameters to run bitextor (learn more about bitextor configuration in the next section), and
* `<NUMJOBS>` is the number of jobs that can be launched in parallel; a job is a single step of the pipeline (see section Pipeline description) and can be run in parallel for different websites
For example, on a machine with 4 cores, one could run Bitextor as follows:
```bash
bitextor.sh -s myconfig.yaml -j 4
```

If bitextor is run on a cluster with a software that allows to manage job queues, two more options can be used: 
```bash
bitextor.sh -s <CONFIGFILE> [-j <NUMJOBS>] [-c <CLUSTERCOMMAND>] [-g <CLUSTERCONFIG>]
```
where
* `<NUMJOBS>` is redefined as the number of jobs that can be submitted to the cluster queue at the same time,
* `<CLUSTERCOMMAND>` is the command that allows to submit a job to a cluster node (for example, this command would be `sbatch` in SLURM or `qsub` in PBS),
* `<CLUSTERCONFIG>` is a JSON configuration file that specifies the specific requirements for each job in the cluster (for example, this file specifies if a job requires a certain amount of RAM memory, or access to one or more GPUs, for example).  Further information about how to configure job requirements in a cluster can be obtained in [Snakemake's documentation](https://snakemake.readthedocs.io/en/stable/snakefiles/configuration.html#cluster-configuration).

### Running Bitextor on a cluster
When running on a cluster with, for example, the [SLURM](https://slurm.schedmd.com/) workload manager installed, one could run Bitextor as:
```bash
bitextor.sh -s myconfig.yaml -j 20 -c "sbatch"
```
This command would run bitextor allowing to submit 20 jobs in the cluster queue at the same time, assuming that all jobs can be run in any node of the cluster.

Now assume that we plan to train a neural MT (NMT) system with Bitextor for document alignment (see next section). In this case, we would need to configure the call to the cluster in a way that those rules that require using GPUs for training or running NMT are run in nodes with GPUs. We could create a cluster configuration file such as the following (extracted from `snakemake/examples/cluster.json`):

```json
{
    "__default__" :
    {
        "gres": "",
    },

    "docaling_translate_nmt" :
    {
        "gres": "--gres gpu:tesla:1"
    },

    "train_nmt_all":
    {
        "gres": "--gres gpu:tesla:1"
    }

}
```
This configuration file tells the cluster to set the option `gres` to empty for all jobs except for `docalign_translate_nmt` and `train_nmt_all` for which it would take value `--gres gpu:tesla:1`. In [SLURM](https://slurm.schedmd.com/) `--gres` is the option that allows to specify a resource when queuing a job; in the example we would be specifying that a Tesla GPU is required by these two jobs. Once we had our configuration file, we could call bitextor in the following way:
```bash
bitextor.sh -s myconfig.yaml -j 20 -c "sbatch {cluster.gres}" -g cluster.json
```
Note that, in this case, an additional option needs to be added to the `sbatch` command so it is called using the specific `gres` option as indicated in the config file `cluster.json` described above: it will be empty for most jobs but for `docalign_translate_nmt` and `train_nmt_all`.

## Bitextor configuration file
Bitextor uses a configuration file to define the variables required by the pipeline. Depending on the options defined in this configuration file the pipeline can behave differently, running alternative tools and functionalities. The following is an exhaustive overview of all the options that can be set in the configuration file and how they affect to the pipeline.

**Suggestion**: A minimalist configuration file sample (`default.yaml`) can be found in this repository (`snakemake/example/tests/default.yaml`). You can take it as an starting point by changing all the paths to match your environment.

### Basic variables
There are a few variables that are mandatory for running bitextor, independently of the task to be carried out:
```yaml
permanentDir: /home/user/permanent/bitextor-output
transientDir: /home/user/transient

lang1: en
lang2: fr
```
* `permanentDir` and `transientDir`: Folders used during processing: `permanentDir` will contain the results of crawling, i.e. the parallel corpus built and the WARC files obtained through crawling; `transientDir` will contain the rest of files generated during processing
* `lang1` and `lang2`: Languages for which parallel data is crawled; note that if MT is used in the pipeline (either for alignment or evaluation) the translation direction used will be lang1 -> lang2

There are some additional options that are rather basic but not mandatory as they take default values if they are not defined
```yaml
bitextor: /home/user/bitextor

LANG1Tokenizer: /home/user/bitextor/preprocess/moses/tokenizer/tokenizer.perl -q -b -a -l en
LANG2Tokenizer: /home/user/bitextor/preprocess/moses/tokenizer/tokenizer.perl -q -b -a -l fr
LANG1SentenceSplitter: /home/user/bitextor/preprocess/moses/ems/support/split-sentences.perl -q -b -l en
LANG2SentenceSplitter: /home/user/bitextor/preprocess/moses/ems/support/split-sentences.perl -q -b -l fr

temp: /home/user/transient

maxSizeWARC: 1000

boilerpipeCleaning: true
alcazar: false
```
* `bitextor`: Directory where Bitextor is installed (the repository or tarball downloaded and compiled); if it is not specified it is assumed that it is the director where the script `bitextor.sh` is
* `LANG1Tokenizer` and `LANG2Tokenizer`: scripts for word-tokenization both for `lang1` and `lang2`. These scripts must read from the standard input and write to the standard output. If no tokenizer is set the one provided by the [Moses](https://github.com/moses-smt/mosesdecoder/blob/master/scripts/ems/support/split-sentences.perl) toolkit is used.
* `LANG1SentenceSplitter` and `LANG2SentenceSplitter`: scripts for sentence splitting both for `lang1` and `lang2`. Again the scripts must read from the standard input and write to the standard output. If no sentence splitter is set the one provided by the [Moses](https://github.com/moses-smt/mosesdecoder/blob/master/scripts/tokenizer/tokenizer.perl) toolkit is used.
* `temp`: temporary directory where some files that will be only needed for a single job will be stored; if it is not defined it is set to the same directory as `transientDir`
* `maxSizeWARC`: when a website is crawled, all the documents downloaded are stored into a WARC file; this option allows to specify the maximum size of a WARC file, so when it is reached the WARC file is split into *n* files containing, as much, the maximum value set. This allows to run pre-processing in parallel for each of the WARC files obtained. Smaller values of this option implies a higher number of WARC files that can be pre-processed in parallel which, depending on the resources available, may result in a faster running of Bitextor
* `boilerpipeCleaning`: option that enables the use of the tool [boilerpipe](https://boilerpipe-web.appspot.com/) to remove boilerplates from HTML documents; by default this is disabled
* `alcazar`: option that enables the library [alcazar](https://github.com/saintamh/alcazar/) for text extraction from HTML documents; by default `lxml` library is used

### Variables defining data sources
The next set of options refer to the source from which data will be crawled. Two options can be specified for crawling: one is to specify a list of websites to be crawled, while the other one is to provide a *langstat* file (see below) containing language statistics regarding the documents in one or more websites, so promising websites can be identified.
```yaml
hosts: ["www.elisabethtea.com","vade-antea.fr"]

langstat: /home/user/langstat/langstats.all.gz
langstatThreshold: 50
langstatExcludeDomains: /home/user/bitextor/snakemake/exclude-domains
```
* `hosts`: list of [hosts](https://en.wikipedia.org/wiki/URL) to be crawled; the host is the part of the URL of a website that identifies the web domain, this is, the URL without the protocol and the path. For example, in the case of the url *https://github.com/bitextor/bitextor* the host would be *github.com*
* `langstat`: file containing language statistics of a collection of websites (hosts). The langstat file is a tab-separated list of tuples *host - language - amount of documents*. For example:
```
0-0hamster.livejournal.com      el      17
0-0hamster.livejournal.com      en      1102
0-0hamster.livejournal.com      hi      19
0-0hamster.livejournal.com      ms      33
0-0hamster.livejournal.com      nn      29
```
* `langstatThreshold`: minimum number of documents in each language so the web domain is considered for crawling.

### Variables for crawling configuration
Three crawlers are supported by Bitextor: one is based on the library [Creepy](https://github.com/Aitjcize/creepy), `wget` tool and [HTTrack](https://www.httrack.com/). The following are the variables that allow to choose one of them and to configure some aspects of the crawling.
```yaml
crawler: httrack

crawlTimeLimit: 30s

crawlSizeLimit: 1G
crawlTld: false
crawlerNumThreads: 1
crawlerConnectionTimeout: 10
```
* `crawler`: set which crawler is used (`wget`,`creepy` or `httrack`)
* `crawlerUserAgent`: [user agent](https://developers.whatismybrowser.com/useragents/explore/software_type_specific/crawler/) to be added to the header of the crawler when doing requests to a web server (identifies your crawler when downloading a website)
* `crawlTimeLimit`: time (in seconds) for which a website can be crawled; for example: *3600s* for a crawl of an hour
* `crawlSizeLimit`: **creepy-specific option** that limits the size of the crawl, i.e. when this limit is reached the crawl ends; it can be specified in GB (G), MB (M) or KB (K)
* `crawlTld`: **creepy-specific option** that allows the crawler to jump to a different web domain as far as it is part of the same [top-level domain](https://en.wikipedia.org/wiki/Top-level_domain) (TLD); a TLD could be, for example, *.es*, *.info* or *.org*
* `crawlerNumThreads`: **creepy-specific option** that allows to specify the number of threads to be be used by the crawler; by default this number is 1
* `crawlerConnectionTimeout`: **creepy-specific option** that allows to specify the connection timeout to a web server

If you want to also crawl PDFs (only `wget` support for now), use these settings:
```
crawler: wget
crawlFileTypes: "html,pdf"
```

### Variables for document alignment
Two strategies are implemented in bitextor for document alignment. The first one uses bilingual lexica to compute word-overlapping-based similarity metrics; these metrics are combined with other features that are extracted from HTML files and used by a linear regressor to obtain a similarity score. The second one uses machine translation (MT) and a [TF/IDF](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) similarity metric computed on the original documents in `lang1` and the translations  of documents in `lang2`. Bitextor allows to build (if necessary) both the bilingual lexica and the MT system from parallel data.

```yaml
documentAligner: DIC
```
The variable `documentAligner` can take three different values, each of them taking a different document-alignment strategy:
* `DIC`: takes the strategy using bilingual lexica and a linear regressor
* `externalMT`: takes the strategy using MT, in this case using an external MT script (provided by the user) that reads source-language text from the standard input and writes the translations to the standard output
* `NMT`: uses parallel data to train a neural MT (NMT) system that is then used for document alignment

#### Variables for document alignment using bilingual lexica
```yaml
dic: /home/user/en-fr.dic
```
Option `dic` specifies the path to the bilingual lexicon to be used for document alignment. If the lexicon specified does not exist, the pipeline will try to build it using a parallel corpus provided through the variable `initCorpusTrainPrefix`:
```yaml
initCorpusTrainPrefix: ['/home/user/Europarl.en-fr.train']
```
This variable must contain one or more **corpus prefixes**. For a given prefix (`/home/user/training` in the example) the pipeline expects to find one file `prefix`.`lang1` and another `prefix`.`lang2` (in the example, `/home/user/Europarl.en-fr.train.en` and `/home/user/Europarl.en-fr.train.fr`). If several training prefixes are provided, the corresponding files will be concatenated before building the bilingual lexicon.

**Suggestion**: a number of pre-built bilingual lexica is available in the repository [bitextor-data](https://github.com/bitextor/bitextor-data/releases/tag/bitextor-v1.0). It is also possible to use other lexica already available, such as those in [OPUS](http://opus.nlpl.eu/), as long as their format is the same as those in the repository.


#### Variables for document alignment using external MT
```yaml
alignerCmd: "example/dummy-translate.sh"
docAlignThreshold: 0.1
```
* `alignerCmd`: command to call the external MT script
* `docAlignThreshold`: threshold for discarding document pairs with a very low TF/IDF similarity score; this option takes values in [0,1] and is 0.0 by default

#### Variables for document alignment using a home-brew neural MT system
If this option is chosen, a Marian NMT model will be trained and evaluated before using it for document alignment. Note that, given the computational cost of training an NMT system, this option requires having a GPU available. The following are mandatory variables in order to build the NMT system:
```yaml
initCorpusTrainPrefix: ['/home/user/Europarl.en-fr.train']
initCorpusDevPrefix: ['/home/user/Europarl.en-fr.dev']
initCorpusTestPrefix: ['/home/user/Europarl.en-fr.test']

marianDir: /home/user/marian-dev
mosesDir: /home/user/mosesdecoder
subwordNmtDir: /home/user/subword-nmt

nmtVocabSize: 50000

LANG2Detokenizer: "/home/user/mosesdecoder/scripts/tokenizer/detokenizer.perl -l fr"

gpuId: 0

marianArgs: [" --optimizer-delay 1", "--mini-batch-fit", "--mini-batch 1000", "--maxi-batch 1000", "--overwrite", "--keep-best", "--valid-metrics perplexity", "--valid-log valid.log", "--log train.log", "--dropout-rnn 0.2", "--dropout-src 0.2", "--dropout-trg 0.2 ", "--cost-type ce-mean-words", "--layer-normalization", "--exponential-smoothing", "--tied-embeddings", "--valid-metrics bleu"]
```
* `initCorpusTrainPrefix`, `initCorpusDevPrefix`,  and `initCorpusTestPrefix`: training data prefixes, development data prefixes and test data prefixes. See section *Variables for document alignment using bilingual lexica* for a description of such prefixes
* `marianDir`: path to the directory containing the installation of the NMT tool [Marian](https://github.com/marian-nmt/marian-dev)
* `mosesDir`: path to the directory containing the MT tool [Moses](https://github.com/moses-smt/mosesdecoder); note that only data pre-processing scripts are used from Moses and, therefore, it is not necessary to compile the project to use it to train and NMT system
* `subwordNmtDir`: path to the directory containing the installation of the tool [subword-nmt](https://github.com/rsennrich/subword-nmt)
* `nmtVocabSize`: size of the NMT vocabulary
* `LANG2Detokenizer`: path to a detokenization script that reads from the standard input and writes to the standard output
* `gpuId`: id of the GPU to be used for training and testing
* `marianArgs`: additional arguments for Marian training

### Options for segment alignment
After document alignment, the next step in the pipeline is segment alignment. This can be carried out by using the tool [hunalign](http://mokk.bme.hu/resources/hunalign/) or the tool [bleualign](https://github.com/rsennrich/Bleualign). The first one uses a bilingual lexicon and is best suited for the `DIC` option of `documentAligner`; the second one uses MT and is only available if one of the options based on MT has been specified in `documentAligner`.
```yaml
bleualign: true
bleuAlignThreshold: 0.1
hunalignThreshold: 0.0
```
* `bleualign`: if this option is set, bleualign is used instead of hunalign as the tool for segment alignment. This option will only work is `documentAligner` is set either to `externalMT` or `NMT`. This option false by default
* `bleuAlignThreshold` and `hunalignThreshold`: score threshold for filtering pairs of sentences with a score too low. `bleuAlignThreshold` should be set to a value in [0,1], while `hunalignThreshold` can take any float value. Both are set to 0.0 by default

### Variables for parallel data filtering
Parallel data filtering is carried out with the tool [Bicleaner](https://github.com/bitextor/bicleaner); this tool uses a pre-trained regression model to filter out pairs of segments with a low confidence score (learn more about Bicleaner at [https://github.com/bitextor/bicleaner]). The options required to make it work are:
```yaml
bicleaner: /home/user/bicleaner-model/en-fr/training.en-fr.yaml
bicleanerThreshold: 0.6
```
* `bicleaner`: path to the YAML configuration file of a pre-trained model. A number of pre-trained models are available at [https://github.com/bitextor/bitextor-data/releases/tag/bicleaner-v1.0]. They are ready to be downloaded and decompressed
* `bicleanerThreshold`: threshold for the confidence score obtained with bitextor to filter low-confidence segment pairs. It is recommended to set it to values in [0.5,0.7], even though it is set to 0.0 by default

If the bicleaner model is not available, the pipeline will try to train one automatically from the data provided through the config file options `initCorpusTrainPrefix` and `bicleanerCorpusTrainingPrefix`:
```yaml
initCorpusTrainPrefix: ['/home/user/Europarl.en-fr.train']
bicleanerCorpusTrainingPrefix: '/home/user/RF.en-fr'
```
* `initCorpusTrainPrefix`: prefix to parallel corpus (see section *Variables for document alignment using bilingual lexica*) that will be used to train statistical dictionaries which are part of the bicleaner model 
* `bicleanerCorpusTrainingPrefix`: prefix to the parallel corpus that will be used to train the regressor that obtains the confidence score in Bicleaner

It is important to provide different parallel corpora for these two options as this helps bicleaner when dealing with unkown words (that do not apear in the statistical dictionaries) during scoring.

### Other post-processing variables
Some other options can be configured to specify the output format of our corpus:
```yaml
restorative: true

elrc: true

tmx: true

deduped: false

deferred: true
```
* `restorative`: if this option is set, the [restorative cleaning](https://github.com/bitextor/restorative-cleaning) is applied to all sentences
* `elrc`: if this option is set, some ELRC quality indicators are added to the final corpus, such as the ratio of target length to source length; these indicators can be used later to filter-out some segment pairs manually
* `tmx`: if this option is set, the output corpus is formatted as a [TMX](https://en.wikipedia.org/wiki/Translation_Memory_eXchange) translation memory
* `deduped`: if this option is set in conjunction with `tmx`, the resulting TMX will not contain repeated segment pairs; if a segment pair is found in more than one pair of documents, it will be provided with more than two URLs, so it is possible to know in which original URLs it appeared
* `deferred`: if this option is set, segment contents (plain text or TMX) are deferred to the original location given a [standoff annotation](https://github.com/lpla/standoff)

NOTE: In case you need to convert a TMX to a tab-separated plain-text file (Moses format), you could use [TMXT](https://github.com/sortiz/tmxt) tool

## Pipeline description

Bitextor is a pipeline that runs a collection of scripts to produce a parallel corpus from a collection of multilingual websites. The pipeline is divided in five stages:
1. **Crawling**: documents are downloaded from the specified websites
2. **Pre-processing**: downloaded documents are normalized, boilerplates are removed, plain text is extracted, and language is identified
3. **Document alignment**: parallel documents are identified. Two strategies are implemented for this stage:
  - one using bilingual lexica and a collection of features extracted from HTML; a linear regressor combines these resources to produce a score in [0,1], and
  - another using machine translation and a [TF/IDF](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) strategy to score document pairs
4. **Segment alignment**: each pair of documents is processed to identify parallel segments. Again, two strategies are implemented:
  - one using the tool [Hunalign](http://mokk.bme.hu/resources/hunalign/), and
  - another using [Bleualign](https://github.com/rsennrich/Bleualign), that can only be used if the MT-based-document-alignment strategy is used (machine translations are used for both methods)
5. **Post-processing**: final steps that allow to clean the parallel corpus obtained using the tool [bicleaner](https://github.com/bitextor/bicleaner), deduplicates translation units, and computes additional quality metrics

The following diagram shows the structure of the pipeline and the different scripts that are used in each stage:

![Banner](https://raw.githubusercontent.com/bitextor/bitextor/master/img/bitextor7.png)
=====

![Connecting Europe Facility](https://www.paracrawl.eu/images/logo_en_cef273x39.png)

All documents and software contained in this repository reflect only the authors' view. The Innovation and Networks Executive Agency of the European Union is not responsible for any use that may be made of the information it contains.
