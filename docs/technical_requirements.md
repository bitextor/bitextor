# Technical requirements to run Bitextor 7.0

This document describes the technical requirements of the tool Bitextor, version 7.0. The following sections try to provide a better understanding of the tool that allow to choose the best software and hardware environment to run it depending on the use case.

## 1- Bitextor technical requirements
Bitextor is a pipeline that carries out a reduction processing: it starts from a given amount of raw data crawled from the Internet and then applies a series of steps that identify parallel data (discarding all useless data) to produce a parallel corpus that will be, in most cases, much smaller than the initial data set. Therefore, it is worth noting that some of the technical requirements described below depend on the amount of raw data to be processed and the amount of actual amount of parallel data.

### 1.1- Minimum requirements
The minimum requirements for running Bitextor 7.0 are:
* **Unix-based operating system**: the tool has been tested on MacOS, Ubuntu 16.04 and 18.04
* **Access to Internet**: only if crawling is required
* **Memory**: 256 MB of RAM
* **Expected running time**: from about 5 to 10 minutes on a desktop computer with an Intel® Core™ i5-7400 CPU — 3.00GHz, with an HDD with writing speed of 150 MB/s and reading speed of 200 MB/s

For a more realistic task, i.e. crawling a website during 12 hours and processing the data downloaded, some of the requirements mentioned above would change:
* **Memory**: 2 GB of RAM
* **Expected running time**: from 1.5 hours to 2.5 hours on a desktop computer with an Intel® Core™ i5-7400 CPU — 3.00GHz, with an HDD with writing speed of 150 MB/s and reading speed of 200 MB/s (on top of the 12 hours of crawling time)

### 1.2- Massive data crawling use case: building Paracrawl2 English-Hungarian corpus
Bitextor 7.0 is being used to produce the parallel corpora that will be released as part of the project [Paracrawl2: "Broader Web-Scale Provision of Parallel Corpora for European Languages"](http://paracrawl.eu). This section describes the requirements to produce one of the medium-size corpora, namely for the English-Hungarian language pair.

This corpus was created by crawling a total of 29,166 webs for 12 hours each of them, resulting in a total of 559 GB (compressed with [xz](https://en.wikipedia.org/wiki/Xz)). Processing was carried out on a super-computing cluster with 30 nodes of 36 cores (hardware threads) and 128 GB of memory each. The cluster used [PBS pro](https://www.pbspro.org/) for job scheduling, which is fully supported by Bitextor 7.0.

The process was carried out in two stages:
* *Crawling*, which took about 18 days (note that some websites were fully crawled in less than 12 hours), and
* *Processing*, which took 9 days, even though the cluster was fully used only during the first 6 days. During the remaining 3 days only about 100 of the largest websites crawled kept being processed, requiring only 4 of the nodes.

## 2- Considerations about the requirements of the different Bitextor modules
Bitextor is a pipeline implemented on the [snakemake](https://snakemake.readthedocs.io/en/stable/) workflow system. Snakemake is a make-like tool that enables an easy parallelisation of the different modules included in Bitextor when processing multiple websites at the same time. For each website to be processed, Bitextor carries out the following list of steps:
1. Crawling documents containing text (HTML, XML, plain text, etc.) from the website
2. Metadata extraction (format, language, encoding, etc.) and pre-processing
3. Document alignment
4. Segment alignment
5. Data filtering
6. TMX generation

The pipeline supports several alternatives for some of these tasks. Depending on the options chosen, some considerations should be taken into account as regards technical requirements. The following subsections focus on the most relevant considerations.

### 2.1- Internet connection
An Internet connection is required in order to access and download the multilingual documents from targeted websites. Given that Bitextor only downloads text-based documents, which are rather small, and due to the fact that Internet crawlers supported apply a waiting time between requests to servers, there is no need to specify a minimum Internet connection.

### 2.2- Machine translation engine
For document alignment, it is possible to use two alternative modules: one that uses machine translation (MT) and one that uses bilingual lexica. If one wants to use the former document-alignment method, it is necessary to have an MT system ready to be used for the target language pair. Note that any corpus-based MT systems may have specific computational requirements, such as large storage for the translation models or availability of GPUs.

Note that this is not actually a requirement since that, as already mentioned, Bitextor implements a second document aligner that uses bilingual dictionaries. This second method is slightly lighter and requires less running time and memory.

### 2.3- Parallelisation
Bitextor is a pipeline implemented using the [snakemake](https://snakemake.readthedocs.io/en/stable/) workflow management system. Snakemake is a make-like system that connects tasks in such a way that the output of each stage of the pipeline is used as as the input of the next one. Snakemake takes care of organising processing steps in the adequate order by building and subsequently traversing a directed acyclic graph (DAG). This also allows to parallelise the pipeline by running tasks that are not interdependent such as, for example, the situation in which more than one website is being processed at the same time.

It is worth noting that, if more than one website is being processed at the same time, the memory used by the pipeline grows accordingly.

### 2.4- Storage
All the files produced by Bitextor are compressed with [xz](https://en.wikipedia.org/wiki/Xz)). Therefore, the stoarage neede is rather small. At the moment of crawling, it is difficult to estimate the amount of raw data that will be downloaded, given that it depends on the size of the website crawled (unless a size or time limit is set). However, once the raw data is downloaded, the worst-case estimation is that the remaining temporary and output files will require three times as much space in the disk. That is, if 1 GB is crawled from a website, it will be necessary to have 3 GB more available.

### 2.5- Translation units de-duplication
One of the few steps that cannot be run independently for each document is translation units de-duplication, as it can only be run once the while corpus has been aligned at the segment level. 

De-duplication consists in discarding repeated translation units, keeping the URLs of the documents where they originally appeared. Therefore, the output of this process is a list of segment pairs and, for each of them, a list of URL pairs. For such process, two possible strategies can be taken:
* sorting the list of translation units (to group those that are repeated), and then process the list linearly discarding repetitions while keeping the URLs, or
* storing the whole list in a hashmap in memory to identify repeated translation units.

Even though option 2 is faster, the translation memory processed may be too large to store all the translation units in memory. In that case, option 1 would be the best choice, only requiring twice the size of disk space available. If this strategy is used, it is worth noting that the I/O speed of the disk in this process will strongly affect the time required to perform the task.

Finally, it is worth noting that this is an optional step, even though it is advisable as it reduces the size of the resulting translation memories.
