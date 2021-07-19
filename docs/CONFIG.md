# Bitextor configuration file

Bitextor uses a configuration file to define the variables required by the pipeline. Depending on the options defined in this configuration file the pipeline can behave differently, running alternative tools and functionalities. The following is an exhaustive overview of all the options that can be set in the configuration file and how they affect to the pipeline.

**Suggestion**: A minimalist [configuration file sample](config/basic.yaml) is provided in this repository. You can take it as an starting point by changing all the paths to match your environment.

Current pipeline constists of the following steps:

* Crawling
* Plain text extraction
* Sharding
* Sentence splitting
* Translation
* Tokenisation (source and translated target)
* Document alignment
* Segment alignment
* Cleaning and filtering

Following is a description of configuration related to each step, as well as basic variables.

## Data storage

There are a few variables that are mandatory for running Bitextor, independently of the task to be carried out, namely the ones related to where final & intermediate files should be stored.

```yaml
permanentDir: ~/permanent/bitextor-output
dataDir: ~/permanent/data
transientDir: ~/transient
tempDir: ~/transient
```

* `permanentDir`: will contain the final results of the run, i.e. the parallel corpus built
* `dataDir`: will contain the results of crawling (WARC files) and files generated during preprocessing (plain text extraction, sharding, sentence splitting, tokenisation and translation), i.e. every step up to document alignment
* `transientDir`: will contain the results of intermediate steps related to document and sentence alignment, as well as cleaning
* `tempDir`: will contain temporary files that are needed by some steps and removed immediately after they are no longer required

### Workflow execution

There are some optional parameters that allow for a finer control of the execution of the pipeline, namely it is possible to configure some jobs to use more than one core; and it is possible to have a partial execution of Bitextor by specifying what step should be final.

```yaml
until: preprocess
parallelWorkers: {translate: 4, docaling: 8, segaling: 8, bicleaner: 2}
profiling: true
```

* `until`: pipeline executes until specified step and stops. The resulting files will not necessarily be in `permanentDir`, they can also be found in `dataDir` or `transientDir` depending on the rule. Allowed values: `crawl`, `preprocess`, `shard`, `split`, `translate`, `tokenise_src`, `tokenise_trg`, `docalign`, `segalign`, `bifixer`, `bicleaner`, `filter`
* `parallelWorkers`: a dictionary specifying the number of cores that should be used for a job. Allowed values: `split`, `translate`, `tokenise_src`, `tokenise_trg`, `docalign`, `segalign`, `bifixer`, `bicleaner`, `sents`
* `profiling`: use `/usr/bin/time` tool to obtain profiling information about each step.

## Data sources

The next set of option srefer to the source from which data will be harvested. It is possible to specify a list of websites to be crawled and/or a list of [WARC](https://iipc.github.io/warc-specifications/specifications/warc-format/warc-1.1) files that contain pre-crawled websites.
Both can be specified either via a list of source directly in the config file, or via a separated gzipped file that contains one source per line.

```yaml
hosts: ["www.elisabethtea.com","vade-antea.fr"]
hostsFile: ~/hosts.gz

warcs: ["/path/to/a.warc.gz", "/path/to/b.warc.gz"]
warcsFile: ~warcs.gz
```

* `hosts`: list of [hosts](https://en.wikipedia.org/wiki/URL) to be crawled; the host is the part of the URL of a website that identifies the web domain, i.e. the URL without the protocol and the path. For example, in the case of the url *<https://github.com/bitextor/bitextor>* the host would be *github.com*
* `hostsFile`: a path to a file that contains a list of hosts to be crawled; in this file each line should contain a single host, written in the format described above.
* `warcs`: specify one or multiple [WARC](https://iipc.github.io/warc-specifications/specifications/warc-format/warc-1.1) files to use; WARC files must contain individually compressed records
* `warcsFile`: a path to a file that contains a list of WARC files to be included in parallel text mining (silimar to `hosts` and `hostsFile`)

## Crawling

Three crawlers are supported by Bitextor: [Heritrix](https://github.com/internetarchive/heritrix3), `wget` tool and [linguacrawl](https://github.com/transducens/linguacrawl/). The following are the variables that allow to choose one of them and to configure some aspects of the crawling.

```yaml
crawler: wget

crawlTimeLimit: 30s

crawlSizeLimit: 1G
crawlTLD: false
crawlerNumThreads: 1
crawlerConnectionTimeout: 10
```

* `crawler`: set which crawler is used (`heritrix`, `wget` or `linguacrawl`)
* `crawlerUserAgent`: [user agent](https://developers.whatismybrowser.com/useragents/explore/software_type_specific/crawler/) to be added to the header of the crawler when doing requests to a web server (identifies your crawler when downloading a website)
* `crawlTimeLimit`: time (in seconds) for which a website can be crawled; for example: *3600s* for a crawl of an hour (`linguacrawl` needs only the quantity, without any suffix)
* `crawlWait`: option that specifies the time that should be waited between the retrievals. It is intended to avoid a web-site to cut the connection of the crawler due too many connections in a low interval of time
* `crawlFileTypes`: **wget-specific/linguacrawl-specific option** that allows to specify the files which we want to retrieve. Both `wget` and `linguacrawl` use the Content-Type in order to search a pattern which matches, so either "html" or "text/html" will retrieve those files with Content-Type "text/html". The [Content-Type header](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type) contains [MIME](https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types) values
* `crawlSizeLimit`: **linguacrawl-specific option** that limits the size of the crawl, i.e. when this limit is reached the crawl ends
* `crawlTLD`: **linguacrawl-specific option** that allows the crawler to jump to a different web domain as far as it is part of the same [top-level domain](https://en.wikipedia.org/wiki/Top-level_domain) (TLD); TLD can be specified as a list (e.g. ['es', 'fr'], ['ca', 'com', 'es'])
* `crawlerNumThreads`: **linguacrawl-specific option** that allows to specify the number of threads to be be used by the crawler; by default this number is 1
* `crawlerConnectionTimeout`: **linguacrawl-specific option** that allows to specify the connection timeout to a web server
* `dumpCurrentCrawl`: **linguacrawl-specific option** that allows to visualize more information about what the crawler is doing, like a 'verbose' option
* `resumePreviousCrawl`: **linguacrawl-specific option** that allows to resume the crawling, but is not trivial to use since Snakemake executes the workflow based on the files which are needed. This option might be used for those cases where the crawling was stopped at the same time that the workflow, or after removing those files which makes necessary the crawler to be executed again (this last option might be difficult to achieve)
* `crawlCat`: **linguacrawl-specific option** that allows to merge all the downloaded WARCs in just one (`linguacrawl` generates one warc per domain/subdomain accordin to the documentation, which does not specify concretely which one). This option will improbe the number of rules of preprocessing to run, but will cause to lose important information like the source of the WARCs. Be aware that this option might be equally as dangerous if enabled in the case of a large crawling since the preprocessing of a very large WARC might even cost more resources (either time or memory) that the processing of thousands of little WARCs
* `crawlCatMaxSize`: **linguacrawl-specific option** that allows to specify a max. size of the merged WARC. If this option is specified, multiple WARCs will be generated where the retrieved WARCs will be being merged, and new WARCs will be used when the max. size has been reached. The unity being used is the byte, so if we want a max. size of 1 KiB, the value which we should set would be 1024
* `crawlMaxFolderTreeDepth`: **linguacrawl-specific option** that allows to specify the max. folder depth for a URL to be taken into account
* `crawlScoutSteps`: **linguacrawl-specific option** that allows to specify the number of documents to be downloaded from a web-site before the scouting criterion is evaluated (one of the most important features of `linguacrawl` is the scout strategy that implements in order to be as efficient as possible)
* `crawlBlackListURL`: **linguacrawl-specific option** that allows to specify a list of domains which will not be taken into account (i.e. they will not be crawled). The default value is: `['wordpress', 'blogspot', 'facebook', 'google', 'wikipedia', 'youtube', 'perehodi', 'twitter', 'instagram']`
* `crawlPrefixFilter`: **linguacrawl-specific option** that allows to avoid resources which begins with a concrete pattern and we know, previously, that is not going to give us useful information

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
crawlBlackListURL: ['wordpress', 'blogspot', 'facebook', 'google', 'wikipedia', 'youtube', 'perehodi', 'twitter', 'instagram']
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

## Preprocessing and sharding

After crawling, the downloaded webs are processed to extract clean text, detect language, etc. The following set of option define how that process is carried out.

After plain text extracion, the extracted data is sharded via [giashard](https://github.com/paracrawl/giashard) in order to create balanced jobs. Crawled websites and WARCs are distributed in shards for a more balanced processing, where each shard contains one or more complete domain(s). Shards are split into batches of specified size to keep memory consumption in check. Document alignemnt works within shards, i.e. all documents in a shard will be compared for document alignment.

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

* `preprocessor`: this options allows to select one of two text extraction tools, `warc2text` (default) or `warc2preprocess`. `warc2text` is faster but less flexibile (less options) than `warc2preprocess`
* `langs`: list of languages that will be processed in addition to `lang1` and `lang2`

Options specific to `warc2preprocess`:

* `langID`: specify the model that should be used for language identification, [`cld2`](https://github.com/CLD2Owners/cld2) (default) or [`cld3`](https://github.com/google/cld3); `cld2` is faster, but `cld3` can be more accurate for certain languages
* `ftfy`: ftfy is a tool that solves encoding errors (disabled by default)
* `cleanHTML`: attempt to remove some parts of HTML that don't contain text (such as CSS, embedded scripts or special tags) before running ftfy, which is a quite slow, in order to improve overall speed; this has an unwanted side effect of removing too much content if the HTML document is malformed (disabled by default)
* `html5lib`: extra parsing with [`html5lib`](https://pypi.org/project/html5lib/), which is slow but the cleanest option and parses the HTML the same way as the modern browsers, which is interesting for broken HTMLs (disabled by default)
* `boilerpipeCleaning`: enable [boilerpipe](https://boilerpipe-web.appspot.com/) to remove boilerplates from HTML documents (disabled by default)
* `parser`: option that selects HTML parsing library for text extraction, [`bs4`](https://www.crummy.com/software/BeautifulSoup/bs4/doc/) (default), [`modest`](https://github.com/rushter/selectolax), `lxml` (uses `html5lib`) or `simple` (very basic HTML tokenizer)
* `PDFextract`: use [PDFExtraxt](https://github.com/bitextor/python-pdfextract) instead of poppler `pdf2html` converter
* `PDFextract_configfile`: set a path for a PDFExtract config file, specially for language models for a better sentence splitting (see [more info](https://github.com/bitextor/pdf-extract/#pdfextractjson))
* `PDFextract_sentence_join_path`: set a path for sentence-join.py script, otherwise, the one included with bitextor will be used
* `PDFextract_kenlm_path`: set path for kenlm binaries
<!-- * `plainTextHashes`: file with plain text MurmurHashes from a previous Bitextor run, so only hashes that are not found in this file are processed in Bitextor. This is useful in case you want to fully recrawl a domain but only process updated content. Works with `bitextor-warc2preprocess` -->

Sharding options:

* `shards`: set number of shards, where a value of 'n' will result in 2^n shards; default is 8 (2^8 shards). `shards: 0` will force all domains to be compared for alignment

* `batches`: batch size in MB; default is 1024. Large batches will increase memory consumption during document alignment, but will reduce time overhead

## Sentence splitting

By default a Python port of [Moses `split-sentences.perl`](https://pypi.org/project/sentence-splitter/) will be used for sentence splitting. This is recommened even without language support, since it is possible to provide custom non-breaking prefixes. External sentence splitter can by used via `sentence-splitters` parameter (less efficient).

Custom sentence splitters must read plain text documents from standard input and write one sentence per line to standard output.

```yaml
sentenceSplitters: {
  'fr': '/home/user/bitextor/preprocess/moses/ems/support/split-sentences.perl -q -b -l fr',
  'default': '/home/user/bitextor/bitextor/example/nltk-sent-tokeniser.py english'
}

customNBPs: {
  'fr': '/home/user/bitextor/myfrenchnbp.txt'
}
```

* `sentenceSplitters`: provide custom scripts for sentence segmentation per language, script specified under `default` will be applied to all lanuages
* `customNBPs`: provide a set of files with custom Non-Breaking Prefixes for the default sentence-splitter; see [already existing files](https://github.com/berkmancenter/mediacloud-sentence-splitter/tree/develop/sentence_splitter/non_breaking_prefixes) for examples

### Tokenisation

[Moses `tokenizer.perl`](https://github.com/moses-smt/mosesdecoder/blob/master/scripts/tokenizer/tokenizer.perl) is the default tokeniser, which is used through an efficient Python wrapper. This is the recommended option unless a language is not supported.

Custom scripts for tokenisation must read sentences from standard input and write the same number of tokenised sentences to standard output.

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

* `wordTokenizers`: scripts for word-tokenization per language, `default` script will be applied to all languages
* `morphologicalAnalysers`: scripts for morphological analysis (lemmatizer/stemmer). It will only be applied to specified languages after tokenisation, or all of them if `default` script is also provided.

## Document alignment

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

* `DIC`: takes the strategy using bilingual lexica and a linear regressor
* `externalMT`: takes the strategy using MT, in this case using an external MT script (provided by the user) that reads source-language text from the standard input and writes the translations to the standard output

### Using bilingual lexica

```yaml
dic: /home/user/en-fr.dic
```

Option `dic` specifies the path to the bilingual lexicon to be used for document alignment. If the lexicon specified does not exist, the pipeline will try to build it using a parallel corpus provided through the variable `initCorpusTrainingPrefix` using `mgiza` tools:

```yaml
initCorpusTrainingPrefix: ['/home/user/Europarl.en-fr.train']
```

This variable must contain one or more **corpus prefixes**. For a given prefix (`/home/user/training` in the example) the pipeline expects to find one file '`prefix`.`lang1`' and another '`prefix`.`lang2`' (in the example, `/home/user/Europarl.en-fr.train.en` and `/home/user/Europarl.en-fr.train.fr`). If several training prefixes are provided, the corresponding files will be concatenated before building the bilingual lexicon.

**Suggestion**: a number of pre-built bilingual lexica is available in the repository [bitextor-data](https://github.com/bitextor/bitextor-data/releases/tag/bitextor-v1.0). It is also possible to use other lexica already available, such as those in [OPUS](http://opus.nlpl.eu/), as long as their format is the same as those in the repository.

<!-- If you are running out of memory in the `mkcls` rule, maybe you should activate original `mkcls` binary instead of `clustercat` interface using:

```yaml
mkcls: true
```
-->

### Using external MT

```yaml
alignerCmd: "example/dummy-translate.sh"
translationDirection: "es2en"
documentAlignerThreshold: 0.1
```

* `alignerCmd`: command to call the external MT script; the MT system must read documents (one sentence per line) from standard input, and write the translation, with the **same number of lines**, to standard output
* `translationDirection`: the direction of the translation system, specified as '`srcLang`2`trgLang`'; default is lang1->lang2
* `documentAlignerThreshold`: threshold for discarding document pairs with a very low TF/IDF similarity score; this option takes values in [0,1] and is 0.1 by default

## Segment alignment

After document alignment, the next step in the pipeline is segment alignment. This can be carried out by using [hunalign](https://github.com/bitextor/hunalign) or [bleualign](https://github.com/bitextor/bleualign-cpp). The first one uses a bilingual lexicon and is best suited for the `DIC` option of `documentAligner`; the second one uses MT and is only available if one of the options based on MT has been specified in `documentAligner`.

```yaml
sentenceAligner: bleualign
sentenceAlignerThreshold: 0.1
```

* `sentenceAligner`: segment aligner tool, `bleualign` or `hunalign`
* `sentenceAlignerThreshold`: threshold for filtering pairs of sentences with a score too low, values in [0,1] range; default is 0.0

## Parallel data filtering

Parallel data filtering is carried out with [Bicleaner](https://github.com/bitextor/bicleaner); this tool uses a pre-trained regression model to filter out pairs of segments with a low confidence score.

A number of pre-trained models for Bicleaner are available [here](https://github.com/bitextor/bicleaner-data/releases/latest). They are ready to be downloaded and decompressed.

The options required to make it work are:

```yaml
bicleaner: /home/user/bicleaner-model/en-fr/training.en-fr.yaml
bicleanerThreshold: 0.6
```

* `bicleaner`: path to the YAML configuration file of a pre-trained model
* `bicleanerThreshold`: threshold to filter low-confidence segment pairs, accepts values in [0,1] range; default is 0.0 (no filtering). It is recommended to set it to values in [0.5,0.7]

If the Bicleaner model is not available, the pipeline will try to train one automatically from the data provided through the config file options `initCorpusTrainingPrefix` and `bicleanerCorpusTrainingPrefix`:

```yaml
initCorpusTrainingPrefix: ['/home/user/Europarl.en-fr.train']
bicleanerCorpusTrainingPrefix: ['/home/user/RF.en-fr']
```

* `initCorpusTrainingPrefix`: prefix to parallel corpus (see [Variables for bilingual lexica](#using-bilingual-lexica)) that will be used to train statistical dictionaries which are part of the Bicleaner model. If `dic` is provided and does not exist, it will be generated; if `dic` is provided and eixts, it should not be replaced, and '`dic`.generated' will be created. If `hunalign` is used, the provided `dic` will be prioritised instead of the generated one

* `bicleanerCorpusTrainingPrefix`: prefix to the parallel corpus that will be used to train the regressor that obtains the confidence score in Bicleaner

It is important to provide different parallel corpora for these two options as this helps Bicleaner when dealing with unknown words (that do not appear in the statistical dictionaries) during scoring.

## Post-processing

Some other options can be configured to specify the output format of the parallel corpus:

```yaml
bifixer: true
elrc: true

tmx: true
deduped: false

biroamer: true
biroamerOmitRandomSentences: true
biroamerMixFiles: ["/home/user/file-tp-mix1", "/home/user/file-to-mix2"]
biroamerImproveAlignmentCorpus: /home/user/Europarl.en-fr.txt
deferred: false
```

* `bifixer`: use [Bifixer](https://github.com/bitextor/bifixer) to fix parallel sentences and tag near-duplicates for removal <!-- When using `bifixer: true` it is possible to specify additional arguments using `bifixerOptions` variable. More information about these arguments in [Bifixer](https://github.com/bitextor/bifixer) repository. -->
* `elrc`: include some ELRC quality indicators in the final corpus, such as the ratio of target length to source length; these indicators can be used later to filter-out some segment pairs manually
* `tmx`: generate a [TMX](https://en.wikipedia.org/wiki/Translation_Memory_eXchange) translation memory of the output corpus
* `deduped`: generate a de-duplicated tmx and regular versions of the corpus; the tmx corpus will contain a list of URLs for the sentence pairs that were found in multiple websites
* `biroamer`: use [Biroamer](https://github.com/bitextor/biroamer) to ROAM (randomize, omit, anonymize and mix) the parallel corpus; in order to use this `tmx: true` or `deduped: true` will be necessary
* `biroamerOmitRandomSentences`: omit close to 10% of the tmx corpus
* `biroamerMixFiles`: use extra sentences to improve anonymization, this option accepts a list of files which will add the stored sentences, the files are expected to be in Moses format
* `biroamerImproveAlignmentCorpus`: an alignment corpus can be provided in order to improve the entities detection; expected to be in Moses format.
* `deferred`: if this option is set, segment contents (plain text or TMX) are deferred to the original location given a Murmurhash2 64bit checksum

NOTE: In case you need to convert a TMX to a tab-separated plain-text file (Moses format), you could use [TMXT](https://github.com/sortiz/tmxt) tool.
