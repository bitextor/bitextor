
![Banner](img/banner.png?raw=true)
=====

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

`bitextor` is a tool to automatically harvest bitexts from multilingual websites. To run it, it is necessary to provide:
1. The source where the parallel data will be searched: one or more [website hostnames](https://en.wikipedia.org/wiki/URL)
2. The two languages on which the user is interested: language IDs must be provided following the [ISO 639-1](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
3. A source of bilingual information between these two languages: either a bilingual lexicon (such as those available at the [bitextor-data repository](https://github.com/bitextor/bitextor-data/tree/master/dics)), a machine translation (MT) system, or a parallel corpus to be used to produce either a lexicon or an MT system (depending on the document-alignment strategy chosen)

## Dependencies

Apart from downloading all submodules of this repository (you can do it with `git clone --recurse-submodules https://github.com/bitextor/bitextor.git` if you are cloning this repo from scratch or in case you are downloading a tarball just do `git submodule update --init --recursive`), there are some external tools that need to be in the path before installing the project. **autotools** and **pkg-config** are necessary for building and installing the project. Tools from **JDK** are needed to run Java dependences (Boilerpipe). In addition, a c++ compiler is required for compiling dependences. **libboost-all-dev** dependence is need to compile `clustercat` and `mgiza` projects. Optionally, **httrack** can be used for crawling if it is installed.

If you are using an apt-like package manager you can run the following command line to install all these dependences:

`sudo apt install automake pkg-config python3 python3-pip libboost-all-dev openjdk-11-jdk`

The two last dependencies (`libboost-all-dev` and `openjdk-11-jdk`) are not mandatory if you are not going to build your own dictionaries and use `boilerpipe`, respectively.

Furthermore, most of the scripts in bitextor are written in Python 3. Because of this, it is necessary to install Python >= 3. All these explained tools are available in most Unix-based operating systems repositories.

Some additional Python libraries are required. They can be installed automatically with the tool pip by runing (use without `sudo` if you are running in a virtualenv):

`sudo pip3 install -r requirements.txt`

As we explained above, the web crawler HTTrack can be used in Bitextor. To do so, first install it by running the command:

`sudo apt install httrack`

This dependency is not mandatory as a second parallel data crawler is provided in Bitextor (Creepy).

## Submodules compilation

To compile all bitextor submodules you will first need to run the script `configure` (if you are downloading the code directly from the repository you will need to run the script `autogen.sh`, which will identify the location of the external tools used. Then the code will be compiled and optionally installed (using `make install`) by means of the command `make`:

`./autogen.sh && make`

## Some frequent installation issues

In some machines equipped with an AMD CPU you may experience some troubles tensorflow 1.8.0 (the version specified in requirements.txt). In case you have installed all the requirements successfully, but when running ./autoconf.sh or ./configure you get an error that says tensorflow is not installed, please, replace current version with version 1.5:
```
sudo pip3 uninstall tensorflow
sudo pip3 install tensorflow==1.5.0
```

## Run

To run Bitextor use the main script bitextor.sh. In general, this script will take two parameters:
```
bitextor.sh -s <CONFIGFILE> [-j <NUMJOBS>]
```
where
* `<CONFIGFILE>` is a [YAML](https://en.wikipedia.org/wiki/YAML) configuration file containing the list of parameters to run bitextor (learn more about bitextor configuration in the next section), and
* `<NUMJOBS>` is the number of jobs that can be launch in launched in parallel (a job may be a single step of the pipeline or the same step for different websites if more than one is specified in the `<CONFIGFILE>`
For example, on a machine with 4 cores, one could run Bitextor as follows:
```
bitextor.sh -s myconfig.yaml -j 4
```

If bitextor is run on a cluster with a software that allows to manage job queues, two more otpions can be used 
```
bitextor.sh -s <CONFIGFILE> [-j <NUMJOBS>] [-c <CLUSTERCOMMAND>] [-g <CLUSTERCONFIG>]
```
where
* `<NUMJOBS>` is redefined as the number of jobs that can be submitted to the cluster queue at the same time,
* `<CLUSTERCOMMAND>` is the command that allows to submit a job to a cluster node (for example, this command would be `sbatch` in SLURM or `qsub` in PBS),
* `<CLUSTERCONFIG>` is a JSON configuration file that allows to specify the specific requirements for each job in the cluster (for example, this file allows to specify if a job requires more RAM memory, or GPUs available, for example). Since bitextor 7.0 is implemented using the [Snakemake](https://snakemake.readthedocs.io/) pipeline manager. Further information about how to configure job requirements in a cluster can be botained in [Snakemake's documentation](https://snakemake.readthedocs.io/en/stable/snakefiles/configuration.html#cluster-configuration).

### Running Bitextor on a cluster
In the case of running on a cluster with, for example, the SLURM workload manager installed, one could run Bitextor as:
```
bitextor.sh -s myconfig.yaml -j 20 -c "sbatch"
```
this command would run bitextor allowing to queue 20 jobs in the cluster queue, assuming that all jobs can be run in any node of the cluster.

Now assume that we plan to train a neural machine translation (NMT) system with bitextor for document alignment. In this case we would need to configure the call to the cluster in a way that those rules that require using GPUs for training or running NMT. We could create a cluster configuration file such as the following (extracted from `snakemake/examples/cluster.json`):

```
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
this configuration file is telling the cluster to set option `gres` empty for all jobs but `docalign_translate_nmt` and `train_nmt_all` for which it would take value `--gres gpu:tesla:1`. In SLURM `--gres` is the option that allows to specify a resource when queuing a job; in the example we would be specifying that a tesla GPU is required by these two rules. Once we had our configuration file, we could call bitextor in the following way:
```
bitextor.sh -s myconfig.yaml -j 20 -c "sbatch {cluster.gres}" -g cluster.json
```
Note that, in this case, an additional option needs to be added to the `sbatch` command so it is called using the specific `gres` option as indicated in the config file `cluster.json` described above: it will be empty for most jobs but for `docalign_translate_nmt` and `train_nmt_all`.

## Bitextor configuration file
Bitextor uses a configuration file to define the variables required by the pipeline. Depending on the options defined in this configuration file

```
# GENERAL OPTIONS
##Directory where Bitextor is installed
bitextor: /home/bitextor/permanent/bitextor
##Folders used during processing: temp is the temporal folder (/tmp by default); permanentDir will contain the results of crawling: the parallel corpus built and the WARC files obtained through crawling; transientDir will contain the rest of files generated during processing
temp: /home/bitextor/transient
permanentDir: /home/bitextor/permanent/bitextor-output
transientDir: /home/bitextor/transient
##Segment spliters and word tokenizers
LANG1Tokenizer: /home/bitextor/permanent/bitextor/preprocess/moses/tokenizer/tokenizer.perl -q -b -a -l en
LANG2Tokenizer: /home/bitextor/permanent/bitextor/preprocess/moses/tokenizer/tokenizer.perl -q -b -a -l fr
LANG1SentenceSplitter: /home/bitextor/permanent/bitextor/preprocess/moses/ems/support/split-sentences.perl -q -b -l en
LANG2SentenceSplitter: /home/bitextor/permanent/bitextor/preprocess/moses/ems/support/split-sentences.perl -q -b -l fr
##Languages for which parallel data is crawled; note that if MT is used in the pipeline (either for alignment or evaluation) the translation direction used will be lang1 -> lang2
lang1: en
lang2: fr


# CRAWLING OPTIONS
##HTTrack activation: if it is not activated, Creepy crawler is used
httrack: true
##Crawling sources: either a list of domains (without http/https) or a langstat file
hosts: ["www.elenacaffe1863.com","vade-retro.fr"]
langstat: /home/bitextor/permanent/langstat/langstats.all.gz
langstatThreshold: 50000000
langstatExcludeDomains: /home/bitextor/permanent/bitextor/snakemake/exclude-domains
# CREEPY CRAWLER SPECIFIC OPTIONS ##
##Crawling time limit in seconds (s)
crawlTimeLimit: 30s
##If this option is enabled the crawler will keep crawling across a whole top-level domain (.es, .com, .fr, etc.)
crawlTld: false
##Crawling size limit
crawlSizeLimit: 1G
##Number of threads used by the Creepy crawler
crawlerNumThreads: 1
##Connection timeout limit (Default 10 seconds)
crawlerConnectionTimeout: 10


# DICTIONARIES
dic: /home/bitextor/permanent/en-fr.dic
##Corpus for bilingual dictionaries and-or SMT/NMT training, if dictionary or models not given, respectively.
initCorpusTrainPrefix: ""
#Corpus for SMT training
initCorpusDevPrefix: ""
initCorpusTestPrefix: ""
##If enabled, boilerpipe is filtered from HTML's
boilerpipeCleaning: true

```

A minimalist configuration file sample (`default.yaml`) can be found in this repository (`snakemake/example/tests/default.yaml`). Change all the paths to match your environment.

Some dictionaries are provided in [bitextor-data](https://github.com/bitextor/bitextor-data) repository, but customised dictionaries can be easily built from parallel corpora as explained in the next section. It is also possible to use other lexicons already available, such as those in [OPUS](http://opus.nlpl.eu/), as long as their format is compatible with the one defined here [in this page of the wiki](https://github.com/bitextor/bitextor/wiki/Build-bilingual-lexicons-from-parallel-corpora).



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
5. **Post-processing**: final steps that allow to clean the parallel corpus obtained using the tool [bicleaner](https://github.com/bitextor/bicleaner), deduplicates translation units, and computes additional quality metrics

The following diagram shows the structure of the pipeline and the different scripts that are used in each stage:

![Banner](img/bitextor7.png?raw=true)
