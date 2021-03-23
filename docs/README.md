
# ![Banner](https://raw.githubusercontent.com/bitextor/bitextor/master/img/banner.png)

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)
[![Chat on Discord](https://camo.githubusercontent.com/b4175720ede4f2621aa066ffbabb70ae30044679/68747470733a2f2f696d672e736869656c64732e696f2f62616467652f636861742d446973636f72642d627269676874677265656e2e737667)](https://discord.gg/etYDaZm)

`Bitextor` is a tool to automatically harvest bitexts from multilingual websites. To run it, it is necessary to provide:

1. The source where the parallel data will be searched: one or more websites (namely, Bitextor needs [website hostnames](https://en.wikipedia.org/wiki/URL) or [WARC files](https://iipc.github.io/warc-specifications/specifications/warc-format/warc-1.1/))
2. The two languages on which the user is interested: language IDs must be provided following the [ISO 639-1](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
3. A source of bilingual information between these two languages: either a bilingual lexicon (such as those available at the [bitextor-data repository](https://github.com/bitextor/bitextor-data/releases/tag/bitextor-v1.0)), a machine translation (MT) system, or a parallel corpus to be used to produce either a lexicon or an MT system (depending on the alignment strategy chosen, see below)

## Docker installation

If you want to easily install Bitextor, just use Docker commands:

```bash
docker pull bitextor/bitextor # download bitextor docker image

docker run -it --name bitextor bitextor/bitextor # create a new container 'bitextor' and open an interactive terminal

docker start bitextor && docker exec -it bitextor bash # run an interactive terminal on an existing 'bitextor' container
```

If you have `snap` package manager in your system, just install Docker using:

```bash
sudo snap install docker
```

Bitextor folder is located at `/opt/bitextor`, with all dependencies and compilations fulfilled.

## Conda installation

Same as with Docker, you can easily install Bitextor using a Conda environment with the following command:

```bash
conda config --add channels conda-forge # It will appear a warning if you already had the channel added
conda install -c bitextor bitextor
```

If you want a concrete version, you can look in the [Anaconda Repository](https://anaconda.org/anaconda/repo) or use the following command:

```bash
conda search -c bitextor bitextor
```

In order to install Miniconda or Anaconda you can follow the instructions of the [official page](https://conda.io/projects/conda/en/latest/user-guide/install/index.html), but if you want to install Miniconda (Linux x64), you should execute the following (it is an interactive installer, so you will need to follow the steps):

```bash
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh
```

Installing Bitextor using Conda will install all optional and mandatory dependencies. Unlike the other installation methods, you will will be able to run Bitextor using `bitextor` or `bitextor.sh` command instead of execute `/path/to/bitextor.sh`. If you have installed Conda in your home directory as it is by default, Bitextor installation will be in `$HOME/miniconda3/envs/YOUR_ENV/` (if Miniconda3), where Bitextor will reside in `bitextor` folder.

Currently we only support Linux x64 for Conda environment.

## Manual installation

### Dependencies

* Download **Bitextor's submodules**.

  ```bash
  # if you are cloning from scratch:
  git clone --recurse-submodules https://github.com/bitextor/bitextor.git

  # otherwise:
  git submodule update --init --recursive
  ```

* **Required packages**

  These are some external tools that need to be in the path before installing the project. If you are using an apt-like package manager you can run the following commands line to install all these dependencies:

  ```bash
  # mandatory:
  sudo apt install python3 python3-venv python3-pip build-essential cmake libboost-all-dev liblzma-dev time curl pigz parallel

  # optional, feel free to skip dependencies for components that you don't expect to use:
  ## wget crawler:
  sudo apt install wget
  ## httrack crawler:
  sudo apt install httrack
  ## warc2text:
  sudo apt install uchardet libuchardet-dev libzip-dev
  ## biroamer:
  sudo apt install libgoogle-perftools-dev libsparsehash-dev
  ## Heritrix, PDFExtract and boilerpipe:
  sudo apt install openjdk-8-jdk
  ## PDFExtract:
  ## PDFExtract also requires protobuf and CLD3 installed (installation instructions below)
  sudo apt install autoconf automake libtool ant maven poppler-utils apt-transport-https ca-certificates gnupg software-properties-common
  ```

* **Golang** packages

  Additionally, Bitextor uses [giashard](https://github.com/paracrawl/giashard) for WARC files preprocessing. To install this tool Golang has to be installed. The latest version can be installed from [here](http://golang.org/dl) or using snap.

  ```bash
  sudo snap install go # or download from http://golang.org/dl
  # build and place the necessary tools in $HOME/go/bin
  go get github.com/paracrawl/giashard/...
  ```

* **Pip** dependencies

  Furthermore, most of the scripts in Bitextor are written in Python 3. Because of this, it is necessary to install Python >= 3. All the tools explained above are available from the repositories of most Unix-like operating systems.

  Some additional Python libraries are required. They can be installed automatically with `pip`. We recommend using a virtual environment to manage Bitextor installation.

  ```bash
  # create virtual environment:
  python3 -m venv /path/to/virtual/environment
  # activate:
  source /path/to/virtual/environment/bin/activate

  # install dependencies in virtual enviroment
  pip3 install --upgrade pip
  pip3 install -r requirements.txt
  # bicleaner:
  pip3 install -r bicleaner/requirements.txt
  git clone https://github.com/kpu/kenlm
  cd kenlm
  python3 -m pip install . --install-option="--max_order 7"
  mkdir -p build && cd build
  cmake .. -DKENLM_MAX_ORDER=7 -DCMAKE_INSTALL_PREFIX:PATH=/your/prefix/path
  make -j all install
  # bifixer:
  pip3 install -r bifixer/requirements.txt
  # biroamer:
  pip3 install -r biroamer/requirements.txt
  python -m spacy download en_core_web_sm
  ```

  If you don't want to install all Python requirements in `requirements.txt` because you don't expect to run some of Bitextor modules, you can comment those `*.txt` in `requirements.txt` and in the previous command.

* [Optional] **Linguacrawl**

  Linguacrawl is a top-level domain (TLD) crawler which can be installed from its [repository](https://github.com/transducens/linguacrawl/) via pip.
  If you decide to use it, be aware that CLD3 is needed first in order to install linguacrawl. Once installed, this crawler can be used independently of bitextor running `linguacrawl config-file.yaml`.

  ```bash
  # linguacrawl:
  # linguacrawl also requires to have protobuf and cld3 installated (instructions below)
  pip3 install git+https://github.com/transducens/linguacrawl.git
  ```

* [Optional] **Heritrix**

  This crawler can be installed unzipping the content of this .zip, so 'bin' folder gets in the "$PATH": <https://github.com/internetarchive/heritrix3/wiki#downloads>.
  After extracting heritrix, [configure](https://github.com/internetarchive/heritrix3/wiki/Heritrix%20Configuration) it and [run](https://github.com/internetarchive/heritrix3/wiki/Running%20Heritrix%203.0%20and%203.1) the web interface.
  This dependency is also not mandatory (in Docker it is located at `/opt/heritrix-3.4.0-SNAPSHOT`).

* [Optional] **Protobuf** and **CLD3**

  CLD3 (Compact Language Detector v3), is a language identification model that can be used optionally during preprocessing. It is also a requirement for PDFExtract and linguacrawl. The requirements for installation are the following:

  ```bash
  # Install protobuf from official repository: https://github.com/protocolbuffers/protobuf/blob/master/src/README.md
  # Maybe you need to uninstall any other protobuf installation in your system (from apt or snap) to avoid compilation issues
  sudo apt-get install autoconf automake libtool curl make g++ unzip
  wget https://github.com/protocolbuffers/protobuf/releases/download/v3.11.4/protobuf-all-3.11.4.tar.gz
  tar -zxvf protobuf-all-3.11.4.tar.gz
  cd protobuf-3.11.4
  ./configure
  make
  make check
  sudo make install
  sudo ldconfig

  pip3 install Cython # Install Cython dependency for cld3
  pip3 install pycld3 # Install cld3 Python fork from https://github.com/bsolomon1124/pycld3
  ```

### Submodules compilation

Bitextor uses CMake for c++ dependencies compilation:

```bash
mkdir build && cd build
cmake ..
make -j
```

Optionally, it is possible to skip the compilation of the dependencies that are not expected to be used:

```bash
mkdir build && cd build
cmake -DSKIP_MGIZA=ON -DSKIP_CLUSTERCAT=ON .. # MGIZA and Clustercat are used for dictionary generation
# other dependencies that can be skipped:
# WARC2TEXT
# DOCALIGN
# BLEUALIGN
# HUNALIGN
# BIROAMER
make -j4
```

#### Some known installation issues

* Depending on the version of *libboost* that you are using given a certain OS version or distribution package from your package manager, you may experience some problems when compiling some of the sub-modules included in Bitextor. If this is the case you can install it manually by running the following commands:

  ```bash
  sudo apt-get remove libboost-all-dev
  sudo apt-get autoremove
  wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
  tar xvf boost_1_75_0.tar.gz
  cd boost_1_75_0/
  ./bootstrap.sh
  ./b2 -j4 --layout=system install || echo FAILURE
  cd ..
  rm -rf boost_1_75_0*
  ```

## Run

To run Bitextor use the main script `bitextor.sh`. In general, this script takes two parameters:

```bash
bitextor.sh -s <CONFIGFILE> [-j <NUMJOBS>]
```

where

* `<CONFIGFILE>` is a [YAML](https://en.wikipedia.org/wiki/YAML) configuration file containing the list of parameters to run Bitextor (learn more about Bitextor configuration in the next section), and
* `<NUMJOBS>` is the number of jobs that can be launched in parallel; a job is a single step of the pipeline (see section Pipeline description) and can be run in parallel for different websites

For example, on a machine with 4 cores, one could run Bitextor as follows:

```bash
bitextor.sh -s myconfig.yaml -j 4
```

If Bitextor is run on a cluster with a software that allows to manage job queues, two more options can be used:

```bash
bitextor.sh -s <CONFIGFILE> [-j <NUMJOBS>] [-c <CLUSTERCOMMAND>] [-g <CLUSTERCONFIG>] [-k] [-n]
```

where

* `<NUMJOBS>` is redefined as the number of jobs that can be submitted to the cluster queue at the same time,
* `<CLUSTERCOMMAND>` is the command that allows to submit a job to a cluster node (for example, this command would be `sbatch` in SLURM or `qsub` in PBS),
* `<CLUSTERCONFIG>` is a JSON configuration file that specifies the specific requirements for each job in the cluster (for example, this file specifies if a job requires a certain amount of RAM memory, or access to one or more GPUs, for example).  Further information about how to configure job requirements in a cluster can be obtained in [Snakemake's documentation](https://snakemake.readthedocs.io/en/stable/snakefiles/configuration.html#cluster-configuration).
* `-k` option is for going on with independent jobs if a Bitextor rule/job fails.
* `-n` option will ignore temp() folder/files declarations. This is useful when running only a part of the workflow, since temp() would lead to deletion of probably needed files by other parts of the workflow.

### Running Bitextor on a cluster

When running on a cluster with, for example, the [SLURM](https://slurm.schedmd.com/) workload manager installed, one could run Bitextor as:

```bash
bitextor.sh -s myconfig.yaml -j 20 -c "sbatch"
```

This command would run Bitextor allowing to submit 20 jobs in the cluster queue at the same time, assuming that all jobs can be run in any node of the cluster.

Now assume that we plan to train a neural MT (NMT) system with Bitextor for document alignment (see next section). In this case, we would need to configure the call to the cluster in a way that those rules that require using GPUs for training or running NMT are run in nodes with GPUs. We could create a cluster configuration file such as the following (extracted from `workflow/examples/cluster.json`):

```json
{
    "__default__" :
    {
        "gres": ""
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

This configuration file tells the cluster to set the option `gres` to empty for all jobs except for `docalign_translate_nmt` and `train_nmt_all` for which it would take value `--gres gpu:tesla:1`. In [SLURM](https://slurm.schedmd.com/) `--gres` is the option that allows to specify a resource when queuing a job; in the example we would be specifying that a Tesla GPU is required by these two jobs. Once we had our configuration file, we could call Bitextor in the following way:

```bash
bitextor.sh -s myconfig.yaml -j 20 -c "sbatch {cluster.gres}" -g cluster.json
```

Note that, in this case, an additional option needs to be added to the `sbatch` command so it is called using the specific `gres` option as indicated in the config file `cluster.json` described above: it will be empty for most jobs but for `docalign_translate_nmt` and `train_nmt_all`.

## Bitextor configuration file

Bitextor uses a configuration file to define the variables required by the pipeline. Depending on the options defined in this configuration file the pipeline can behave differently, running alternative tools and functionalities. The following is an exhaustive overview of all the options that can be set in the configuration file and how they affect to the pipeline.

**Suggestion**: A minimalist configuration file sample (`basic.yaml`) can be found in this repository (`workflow/sample-config/basic.yaml`). You can take it as an starting point by changing all the paths to match your environment.

Current pipeline constists of the following steps:

* Crawling
* Preprocessing
* Sharding
* Sentence splitting
* Translation
* Tokenisation (source and translated target)
* Document alignment
* Segment alignment
* Cleaning and filtering

Following is a description of configuration related to each step, as well as basic variables.

### Basic variables

There are a few variables that are mandatory for running Bitextor, independently of the task to be carried out:

```yaml
bitextor: /home/user/bitextor

permanentDir: /home/user/permanent/bitextor-output
dataDir: /home/user/permanent/data
transientDir: /home/user/transient

profiling: true
```

* `bitextor`: Directory where Bitextor is installed (the repository or tarball downloaded and compiled).
* `permanentDir`, `transientDir` and `dataDir`: Folders used during processing: `permanentDir` will contain the final results of the run, i.e. the parallel corpus built; `dataDir` will contain the results of crawling (WARC files) and files generated during preprocessing, `transientDir` will contain the rest of files generated in the pipeline.
* `profiling`: use `/usr/bin/time` tool to obtain profiling information about each step.

### Data sources

The next set of options refer to the source from which data will be crawled. Three options can be specified for crawling: one is to specify a list of websites to be crawled in the config file, another one is defining a list of websites in a separated gzipped file, while the last one is to provide a *langstat* file (see below) containing language statistics regarding the documents in one or more websites, so promising websites can be identified.

```yaml
hosts: ["www.elisabethtea.com","vade-antea.fr"]
hostsFile: /home/user/hosts.gz

warcs: ["/home/user/a.warc.gz", "/home/user/b.warc.gz"]
warcsFile: /home/user/warcs.gz
```

* `hosts`: list of [hosts](https://en.wikipedia.org/wiki/URL) to be crawled; the host is the part of the URL of a website that identifies the web domain, this is, the URL without the protocol and the path. For example, in the case of the url *<https://github.com/bitextor/bitextor>* the host would be *github.com*
* `hostsFile`: a path to a file that contains a list of hosts to be crawled; in this file each line should contain a single host, written in the format described above.
* `warcs`: specify one or multiple [WARC](https://iipc.github.io/warc-specifications/specifications/warc-format/warc-1.1) files to use. This option allows to  a define a list of gz compressed WARC files (each record compressed individually), which will be used to extract parallel data.
* `warcsFile`: a path to a file that contains a list of WARC files to be included in parallel text mining (silimar to `hosts` and `hostsFile`)

### Crawling

Five crawlers are supported by Bitextor: one is based on the library [Creepy](https://github.com/Aitjcize/creepy), [Heritrix](https://github.com/internetarchive/heritrix3), `wget` tool, [HTTrack](https://www.httrack.com/) and [linguacrawl](https://github.com/transducens/linguacrawl/). The following are the variables that allow to choose one of them and to configure some aspects of the crawling.

```yaml
crawler: wget

crawlTimeLimit: 30s

crawlSizeLimit: 1G
crawlTLD: false
crawlerNumThreads: 1
crawlerConnectionTimeout: 10

onlyConcat: false
```

* `crawler`: set which crawler is used (`heritrix`, `wget`,`creepy`, `httrack` or `linguacrawl`)
* `crawlerUserAgent`: [user agent](https://developers.whatismybrowser.com/useragents/explore/software_type_specific/crawler/) to be added to the header of the crawler when doing requests to a web server (identifies your crawler when downloading a website)
* `crawlTimeLimit`: time (in seconds) for which a website can be crawled; for example: *3600s* for a crawl of an hour (`linguacrawl` needs only the quantity, without any suffix)
* `crawlWait`: option that specifies the time that should be waited between the retrievals. It is intended to avoid a web-site to cut the connection of the crawler due too many connections in a low interval of time
* `crawlFileTypes`: **wget-specific/linguacrawl-specific option** that allows to specify the files which we want to retrieve. Both `wget` and `linguacrawl` use the Content-Type in order to search a pattern which matchs, so either "html" or "text/html" will retrieve those files with Content-Type "text/html". The [Content-Type header](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type) contains [MIME](https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types) values
* `crawlSizeLimit`: **creepy-specific/linguacrawl-specific option** that limits the size of the crawl, i.e. when this limit is reached the crawl ends; it can be specified in GB (G), MB (M) or KB (K) (in the case of `Creepy`, but not for `linguacrawl`)
* `crawlTLD`: **creepy-specific/linguacrawl-specific option** that allows the crawler to jump to a different web domain as far as it is part of the same [top-level domain](https://en.wikipedia.org/wiki/Top-level_domain) (TLD); a TLD could be, for example, *.es*, *.info* or *.org* for `Creepy`. In the case of `linguacrawl` the TLD can be specified directly without wildcards and in a list (e.g. ['es', 'fr'], ['ca', 'com', 'es'])
* `crawlerNumThreads`: **creepy-specific/linguacrawl-specific option** that allows to specify the number of threads to be be used by the crawler; by default this number is 1
* `crawlerConnectionTimeout`: **creepy-specific/linguacrawl-specific option** that allows to specify the connection timeout to a web server
* `dumpCurrentCrawl`: **creepy-specific/linguacrawl-specific option** that allows to visualize more information about what the crawler is doing, like a 'verbose' option
* `resumePreviousCrawl`: **creepy-specific/linguacrawl-specific option** that allows to resume the crawling, but is not trivial to use since Snakemake executes the workflow based on the files which are needed. This option might be used for those cases where the crawling was stopped at the same time that the workflow, or after removing those files which makes necessary the crawler to be executed again (this last option might be difficult to achieve)
* `crawlCat`: **linguacrawl-specific option** that allows to merge all the downloaded WARCs in just one (`linguacrawl` generates one warc per domain/subdomain accordin to the documentation, which does not specify concretely which one). This option will improbe the number of rules of preprocessing to run, but will cause to lose important information like the source of the WARCs. Be aware that this option might be equally as dangerous if enabled in the case of a large crawling since the preprocessing of a very large WARC might even cost more resources (either time or memory) that the processing of thousands of little WARCs
* `crawlCatMaxSize`: **linguacrawl-specific option** that allows to specify a max. size of the merged WARC. If this option is specified, multiple WARCs will be generated where the retrieved WARCs will be being merged, and new WARCs will be used when the max. size has been reached. The unity being used is the byte, so if we want a max. size of 1 KiB, the value which we should set would be 1024
* `crawlMaxFolderTreeDepth`: **linguacrawl-specific option** that allows to specify the max. folder depth for a URL to be taken into account
* `crawlScoutSteps`: **linguacrawl-specific option** that allows to specify the number of documents to be downloaded from a web-site before the scouting criterion is evaluated (one of the most important features of `linguacrawl` is the scout strategy that implements in order to be as efficient as possible)
* `crawlBlackListURL`: **linguacrawl-specific option** that allows to specify a list of domains which will not be taken into account (i.e. they will not be crawled). The default value is: 'wordpress','blogspot','facebook','google','wikipedia','youtube','perehodi','twitter','instagram'
* `crawlPrefixFilter`: **linguacrawl-specific option** that allows to avoid resources which begins with a concrete pattern and we know, previously, that is not going to give us useful information
* `onlyConcat`: stop Bitextor after the crawling step and group WARC files by domain

If you want to also crawl PDFs (only `wget` support for now), use these settings:

```yaml
crawler: wget
crawlFileTypes: "html,pdf"
```

If you want to use `heritrix` crawler, you should provide the installation folder of `heritrix` and optionally the url (default is 'localhost:8443') and the user:password (default is 'admin:admin'):

```yaml
crawler: heritrix
heritrixPath: /home/user/heritrix-3.4.0-20190418
heritrixUrl: "https://localhost:8443"
heritrixUser: "admin:admin"
```

Heritrix crawler will check if there is a checkpoint in its 'jobs' folder and resume from the latest. If crawl takes longer than the crawl time limit, it will automatically create a checkpoint for a future incremental crawl.

Other option might be `linguacrawl`, which shares multiple configuration options with the rest of crawlers, but there are also unique options for its own configuration:

```yaml
crawler: linguacrawl
crawlFileTypes: text/html,application/pdf
crawlTLD: ["fr", "en"]
crawlSizeLimit: "1024"
crawlCat: true
crawlCatMaxSize: 1024
crawlMaxFolderTreeDepth: "20"
crawlScoutSteps: "200"
crawlBlackListURL: ['wordpress','blogspot','facebook','google','wikipedia','youtube','perehodi','twitter','instagram']
crawlPrefixFilter: ['mailto:']
crawlerNumThreads: "12"
crawlerConnectionTimeout: "3600"
dumpCurrentCrawl: true
resumePreviousCrawl: false
```

If `linguacrawl` is used, a YAML file is created on the fly in order to use it as configuration file, and you can check this file out to be sure that is configured as you want. There are multiple options which are provided with a default value if none was set, so might be interesting to visualize the generated YAML configuration file if you want a concrete behaviour or something is not working as you expected. Those default values are set because are mandatory for `linguacrawl`. Other default behaviour which should be taken into account is:

* Default User Agent is used: Mozilla/5.0 (compatible; Bitextor/8 + <https://github.com/bitextor/bitextor>)
* Default URL blacklist is used if not specified any (you can specify "[]" if you do not want any element in the blacklist): ['wordpress','blogspot','facebook','google','wikipedia','youtube','perehodi','twitter','instagram']
* Default prefix filter is used if not specified any (you can specify "[]" if you do not want any element in the prefix filter list): ['mailto:']
* A maximum of 3 attempts will be made in order to download a resource.
* The number of minimum languages in a site will be 2, and in the case of not satisfy this condition, the site will be discarted
* The mandatory lang to be found in a site will be determined by `lang1` or `lang2` if not defined. A minimum of 10% of content has to be present in the mandatory language in order to not discard a resource
* The accepted TLDs will be those specified in `lang1`, `lang2`, `langs` and `crawlTLD`
* `linguacrawl` works efficiently when multiple hosts are provided, and due to this, we provide all the hosts which are specified in `hosts` and `hostsFile`. This fact makes that we lose some information, like the main source of the links that are crawled in a concrete host of the ones which were provided in the configuration file, but we make a more efficient crawling. This behaviour is different from the rest the crawlers, where each of them execute a different instance with a host
* WARNING: if you use linguacrawl in a cluster, it is highly recommended to use `crawlCat` and `crawlCatMaxSize` in order to balance the work (it is not usual to use a crawler in a cluster)

### Preprocessing and sharding

After crawling, the downloaded web are processed to extract clean text, detect language, etc. The following set of option define how that process is carried out. After preprocessing, the extracted data is sharded via [giashard](https://github.com/paracrawl/giashard).

```yaml
# preprocessing
preprocessor: warc2text
langs: [en, es, fr]

## with warc2preprocess only
boilerpipeCleaning: true
parser: "bs4"
ftfy: false
cleanHTML: false
langID: cld2

# sharding
shards: 8 # 2^8 shards
batches: 1024 # batches of up to 1024MB
```

* `preprocessor`: this options allows to select one of two text extraction tools. Options: `warc2preprocess` and `warc2text`. `warc2text` is the default option, it is faster but currently unable to process PDFs.
* `langs`: a list of languages that will be processed during the preprocessing step. When this option is empty, only LANG1 and LANG2 will be processed during this step. NOTE: if `warc2text`is enabled, every language will be processed, but only languages specified in `langs` will move on to sentence splitting
* `langID`: specify the model that should be used for language identification. Options are [`cld2`](https://github.com/CLD2Owners/cld2) (default) and [`cld3`](https://github.com/google/cld3). Note that `cld2` is faster, but `cld3` can be more accurate for certain languages. `warc2text` uses `cld2`
* `ftfy`: ftfy is a tool that solves encoding errors. Disabled by default
* `cleanHTML`: cleaning HTML takes place before parsing, and the point of this step is to remove some parts of HTML that don't contain text (such as CSS, embedded scripts or special tags) before running ftfy, which is a quite slow. This has an unwanted side effect of removed too much content if the HTML document is malformed. So, enable this step if you want to gain time at the risk of losing some text
* `html5lib`: extra parse with `html5lib`, which is slow but the cleanest option and parses the HTML the same way as the modern browsers, which is interesting for broken HTMLs.
* `boilerpipeCleaning`: option that enables the use of the tool [boilerpipe](https://boilerpipe-web.appspot.com/) to remove boilerplates from HTML documents; by default this is disabled. NOTE: this option does not do anything with `warc2text: true`
* `parser`: option that selects HTML parsing library for text extraction; Options are ['bs4'](https://www.crummy.com/software/BeautifulSoup/bs4/doc/), ['modest'](https://github.com/rushter/selectolax), 'lxml' (which uses `html5lib` parsed tree to recursively extract text from tags, so it forces the `html5lib` option) or 'simple', which is an HTML tokenizer built with [HTMLParser](https://docs.python.org/3/library/html.parser.html). NOTE: does not do anything `warc2text: true`
* `PDFextract`: set to 'true' to use it instead of system native poppler `pdf2html` converter
* `PDFextract_configfile`: set a path for a PDFExtract config file, specially for language models for a better sentence splitting (see [more info](https://github.com/bitextor/pdf-extract/#pdfextractjson))
* `PDFextract_sentence_join_path`: set a path for sentence-join.py script, otherwise, the one included with bitextor will be used
* `PDFextract_kenlm_path`: set path for kenlm binaries
<!-- * `plainTextHashes`: file with plain text MurmurHashes from a previous Bitextor run, so only hashes that are not found in this file are processed in Bitextor. This is useful in case you want to fully recrawl a domain but only process updated content. Works with `bitextor-warc2preprocess` -->
* `shards`: domains and WARCs are distributed in shards for a more balanced processing, and all documents in a shard will be compared for document alignment. Each shard contain one or more complete domains/WARCs. The parameter sets the number os shards (as 'n' in 2^n), being 8 the default (2^8 shards). If you set it to zero with `shards: 0` it will be forcing all domains and WARCs provided to Bitextor to be compared for alignment.
* `batches`: shards are split into batches for parallelization. These shards can divide a domain or WARC, so this is only used in steps that can work with this division, like document aligner. This configuration parameter set the batch size in MB, being 1024 by default.

### Sentence splitting

By default a Python port of [Moses `split-sentences.perl`](https://pypi.org/project/sentence-splitter/) will be used for sentence splitting. This is recommened even without language support, since it is possible to provide custom non-breaking prefixes. External sentence splitter can by used via `sentence-splitters` parameter (less efficient).

```yaml
sentenceSplitters: {
  'fr': '/home/user/bitextor/preprocess/moses/ems/support/split-sentences.perl -q -b -l fr',
  'default': '/home/user/bitextor/workflow/example/nltk-sent-tokeniser.py english'
}

customNBPs: {
  'fr': '/home/user/bitextor/myfrenchnbp.txt'
}
```

* `sentenceSplitters`: scripts for sentence splitting. All the scripts must read from the standard input and write to the standard output. When not specified, [python Moses](https://pypi.org/project/sentence-splitter) will be used.
* `customNBPs`: provide a set of files with custom Non-Breaking Prefixes for the default sentence-splitter. See their format by checking the [already existing files](https://github.com/berkmancenter/mediacloud-sentence-splitter/tree/develop/sentence_splitter/non_breaking_prefixes).

### Tokenisation

[Moses `tokenizer.perl`](https://github.com/moses-smt/mosesdecoder/blob/master/scripts/tokenizer/tokenizer.perl) is the default tokeniser, which is used through an efficient Python wrapper. This is the recommended option unless a language is not supported.

```yaml
wordTokenizers: {
  'fr': '/home/user/bitextor/mytokenizer -l fr',
  'default': '/home/user/bitextor/moses/tokenizer/my-modified-tokenizer.perl -q -b -a -l en'
}
morphologicalAnalysers: {
  'lang1': 'path/to/morph-analyser1',
  'lang2': 'path/to/morph-analyser2'
}
```

* `wordTokenizers`: scripts for word-tokenization. These scripts must read from the standard input and write to the standard output.
* `morphologicalAnalysers`: scripts for morphological analysis (lemmatizer/stemmer). It will only be applied to specified languages after tokenisation, or all of them if `default` script is also provided.

### Document alignment

From this step forward, bitextor works with a pair of languages, which are specified through `lang1` and `lang2` parameters. The output will contain the sentence pairs in that order.

```yaml
lang1: es
lang2: en
```

Two strategies are implemented in Bitextor for document alignment. The first one uses bilingual lexica to compute word-overlapping-based similarity metrics; these metrics are combined with other features that are extracted from HTML files and used by a linear regressor to obtain a similarity score. The second one uses an external machine translation (MT) and a [TF/IDF](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) similarity metric computed on the original documents in one of the languages, and the translation of the documents of the other language.

```yaml
documentAligner: externalMT
```

The variable `documentAligner` can take two different values, each of them taking a different document-alignment strategy:

<!-- The variable `documentAligner` can take three different values, each of them taking a different document-alignment strategy: -->

* `DIC`: takes the strategy using bilingual lexica and a linear regressor.
* `externalMT`: takes the strategy using MT, in this case using an external MT script (provided by the user) that reads source-language text from the standard input and writes the translations to the standard output
<!-- * `NMT`: uses parallel data to train a neural MT (NMT) system that is then used for document alignment -->

#### Using bilingual lexica

```yaml
dic: /home/user/en-fr.dic
```

Option `dic` specifies the path to the bilingual lexicon to be used for document alignment. If the lexicon specified does not exist, the pipeline will try to build it using a parallel corpus provided through the variable `initCorpusTrainingPrefix` using `mgiza` tools:

```yaml
initCorpusTrainingPrefix: ['/home/user/Europarl.en-fr.train']
```

This variable must contain one or more **corpus prefixes**. For a given prefix (`/home/user/training` in the example) the pipeline expects to find one file `prefix`.`lang1` and another `prefix`.`lang2` (in the example, `/home/user/Europarl.en-fr.train.en` and `/home/user/Europarl.en-fr.train.fr`). If several training prefixes are provided, the corresponding files will be concatenated before building the bilingual lexicon.

**Suggestion**: a number of pre-built bilingual lexica is available in the repository [bitextor-data](https://github.com/bitextor/bitextor-data/releases/tag/bitextor-v1.0). It is also possible to use other lexica already available, such as those in [OPUS](http://opus.nlpl.eu/), as long as their format is the same as those in the repository.

If you are running out of memory in the `mkcls` rule, maybe you should activate original `mkcls` binary instead of `clustercat` interface using:

```yaml
mkcls: true
```

#### Using external MT

```yaml
alignerCmd: "example/dummy-translate.sh"
translationDirection: "es2en"
documentAlignerThreshold: 0.1
documentAlignerWorkers: 2
```

* `alignerCmd`: command to call the external MT script
* `translationDirection`: the direction of the translation system, default is lang1->lang2
* `documentAlignerThreshold`: threshold for discarding document pairs with a very low TF/IDF similarity score; this option takes values in [0,1] and is 0.1 by default

<!---
#### Using a home-brew neural MT system

If this option is chosen, a Marian NMT model will be trained and evaluated before using it for document alignment. Note that, given the computational cost of training an NMT system, this option requires having a GPU available. The following are mandatory variables in order to build the NMT system:

```yaml
initCorpusTrainingPrefix: ['/home/user/Europarl.en-fr.train']
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

* `initCorpusTrainingPrefix`, `initCorpusDevPrefix`,  and `initCorpusTestPrefix`: training data prefixes, development data prefixes and test data prefixes. See section *Variables for document alignment using bilingual lexica* for a description of such prefixes
* `marianDir`: path to the directory containing the installation of the NMT tool [Marian](https://github.com/marian-nmt/marian-dev)
* `mosesDir`: path to the directory containing the MT tool [Moses](https://github.com/moses-smt/mosesdecoder); note that only data pre-processing scripts are used from Moses and, therefore, it is not necessary to compile the project to use it to train and NMT system
* `subwordNmtDir`: path to the directory containing the installation of the tool [subword-nmt](https://github.com/rsennrich/subword-nmt)
* `nmtVocabSize`: size of the NMT vocabulary
* `LANG2Detokenizer`: path to a detokenization script that reads from the standard input and writes to the standard output
* `gpuId`: id of the GPU to be used for training and testing
* `marianArgs`: additional arguments for Marian training
--->

### Segment alignment

After document alignment, the next step in the pipeline is segment alignment. This can be carried out by using the tool [hunalign](http://mokk.bme.hu/resources/hunalign/) or the tool [bleualign](https://github.com/rsennrich/Bleualign). The first one uses a bilingual lexicon and is best suited for the `DIC` option of `documentAligner`; the second one uses MT and is only available if one of the options based on MT has been specified in `documentAligner`.

```yaml
sentenceAligner: bleualign
sentenceAlignerThreshold: 0.1
sentenceAlignerWorkers: 1
```

* `sentenceAligner`: segment aligner tool which is going to be used. Default is `bleualign`, but `hunalign` can be used in order to achieve a dictionary-based alignment. If `bleualign` is used, `documentAligner: externalMT` is mandatory, but in the case of `hunalign`, both `externalMT` and `DIC` are allowed as document aligner.
* `sentenceAlignerThreshold`: score threshold for filtering pairs of sentences with a score too low. The value should be set to a value in [0,1] (both `bleualign` and `hunalign`). The default value is 0.0.

### Parallel data filtering

Parallel data filtering is carried out with the tool [Bicleaner](https://github.com/bitextor/bicleaner); this tool uses a pre-trained regression model to filter out pairs of segments with a low confidence score (learn more about Bicleaner [here](https://github.com/bitextor/bicleaner)). The options required to make it work are:

```yaml
bicleaner: /home/user/bicleaner-model/en-fr/training.en-fr.yaml
bicleanerThreshold: 0.6
```

* `bicleaner`: path to the YAML configuration file of a pre-trained model. A number of pre-trained models are available [here](https://github.com/bitextor/bicleaner-data/releases/latest). They are ready to be downloaded and decompressed
* `bicleanerThreshold`: threshold for the confidence score obtained with bitextor to filter low-confidence segment pairs. It is recommended to set it to values in [0.5,0.7], even though it is set to 0.0 by default

If the Bicleaner model is not available, the pipeline will try to train one automatically from the data provided through the config file options `initCorpusTrainingPrefix` and `bicleanerCorpusTrainingPrefix`:

```yaml
initCorpusTrainingPrefix: ['/home/user/Europarl.en-fr.train']
bicleanerCorpusTrainingPrefix: ['/home/user/RF.en-fr']
```

* `initCorpusTrainingPrefix`: prefix to parallel corpus (see section *Variables for document alignment using bilingual lexica*) that will be used to train statistical dictionaries which are part of the Bicleaner model. This option affects and is affected by `dic` config option, which in case that provided `dic` does not exist, a new one will be generated; in case that provided `dic` does exist, should not be replaced (it does might!) but a new one would be generated and stored in "`dic`.generated". If `sentenceAligner: hunalign` is being used and this situation happens, the provided and existant `dic` will be used in the main pipeline while the new dictionary will only be used in order to train the Bicleaner model
* `bicleanerCorpusTrainingPrefix`: prefix to the parallel corpus that will be used to train the regressor that obtains the confidence score in Bicleaner

It is important to provide different parallel corpora for these two options as this helps Bicleaner when dealing with unknown words (that do not appear in the statistical dictionaries) during scoring.

### Post-processing

Some other options can be configured to specify the output format of our corpus:

```yaml
bifixer: true
elrc: true
tmx: true
deduped: false
deferred: false
```

* `bifixer`: if this option is set, [Bifixer](https://github.com/bitextor/bifixer) is used to fix parallel sentences and tag near-duplicates for removal. When using `bifixer: true`, it is possible to specify additional arguments using `bifixerOptions` variable. More information about these arguments in [Bifixer](https://github.com/bitextor/bifixer) repository.
* `elrc`: if this option is set, some ELRC quality indicators are added to the final corpus, such as the ratio of target length to source length; these indicators can be used later to filter-out some segment pairs manually
* `tmx`: if this option is set, the output corpus is formatted as a [TMX](https://en.wikipedia.org/wiki/Translation_Memory_eXchange) translation memory
* `deduped`: if this option is set in conjunction with `tmx`, the resulting TMX will not contain repeated segment pairs; if a segment pair is found in more than one pair of documents, it will be provided with more than two URLs, so it is possible to know in which original URLs it appeared
* `deferred`: if this option is set, segment contents (plain text or TMX) are deferred to the original location given a Murmurhash2 64bit checksum. 

NOTE: In case you need to convert a TMX to a tab-separated plain-text file (Moses format), you could use [TMXT](https://github.com/sortiz/tmxt) tool

#### Biroamer

In order to ROAM the resulted TMX (either normal or deduped), you can use some options to configure the result:

```yaml
biroamer: true
biroamerOmitRandomSentences: true
biroamerMixFiles: ["/home/user/file-tp-mix1", "/home/user/file-to-mix2"]
biroamerImproveAlignmentCorpus: /home/user/Europarl.en-fr.txt
```

* `biroamer`: through this option we enable the ROAM feature. In order to ROAM the resulted TMX, [Biroamer](https://github.com/bitextor/biroamer) is used. If this option is set to 'true', `tmx: true` or `deduped: true` will be necessary.
* `biroamerOmitRandomSentences`: in order to omit random sentences, this option can be used. The quantity of sentences removed are close to 10% of the original TMX.
* `biroamerMixFiles`: when is necessary to add external sentences to improve anonymization, this option accepts a list of files which will add the stored sentences. The files are expected to be in morse format, and those files will be concatenated.
* `biroamerImproveAlignmentCorpus`: an alignment corpus can be provided in order to improve the entities detection. The corpus file is exteced to be in morse format.

## Pipeline description

Bitextor is a pipeline that runs a collection of scripts to produce a parallel corpus from a collection of multilingual websites. The pipeline is divided in five stages:

1. **Crawling**: documents are downloaded from the specified websites
2. **Pre-processing**: downloaded documents are normalized, boilerplates are removed, plain text is extracted, and language is identified
3. **Document alignment**: parallel documents are identified. Two strategies are implemented for this stage:
    * one using bilingual lexica and a collection of features extracted from HTML; a linear regressor combines these resources to produce a score in [0,1], and
    * another using machine translation and a [TF/IDF](https://en.wikipedia.org/wiki/Tf%E2%80%93idf) strategy to score document pairs
4. **Segment alignment**: each pair of documents is processed to identify parallel segments. Again, two strategies are implemented:
    * one using the tool [Hunalign](http://mokk.bme.hu/resources/hunalign/), and
    * another using [Bleualign](https://github.com/rsennrich/Bleualign), that can only be used if the MT-based-document-alignment strategy is used (machine translations are used for both methods)
5. **Post-processing**: final steps that allow to clean the parallel corpus obtained using the tool [Bicleaner](https://github.com/bitextor/bicleaner), deduplicates translation units, and computes additional quality metrics

The following diagram shows the structure of the pipeline and the different scripts that are used in each stage:

![Banner](https://raw.githubusercontent.com/bitextor/bitextor/master/img/bitextor.png)

![Connecting Europe Facility](https://raw.githubusercontent.com/bitextor/bitextor/master/img/logo_en_cef273x39_nonalpha.png)

All documents and software contained in this repository reflect only the authors' view. The Innovation and Networks Executive Agency of the European Union is not responsible for any use that may be made of the information it contains.
