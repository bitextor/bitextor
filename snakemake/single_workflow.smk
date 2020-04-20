import os
import sys

include: "utils.smk"

validate_args(config)

sys.path.append(os.path.dirname(os.path.abspath(config["bitextor"]) + "/utils"))
from utils.common import open_xz_or_gzip_or_plain

#################################################################
# BASIC PARAMETERS
BITEXTOR = config["bitextor"]
DATADIR = config["dataDir"]
TRANSIENT = config["transientDir"]
PERMANENT = config["permanentDir"]
TMPDIR = config["transientDir"]
if "tempDir" in config:
    TMPDIR = config["tempDir"]

LANGS = set()
LANG1 = ""
LANG2 = ""

if "langs" in config:
    LANGS = set(config["langs"])
if "lang1" in config:
    LANG1 = config["lang1"]
    LANGS.add(LANG1)
if "lang2" in config:
    LANG2 = config["lang2"]
    LANGS.add(LANG2)

ONLY_PREPROCESS = False
ONLY_CRAWL = False
if "onlyCrawl" in config and config["onlyCrawl"]:
    ONLY_CRAWL = True
if "onlyPreprocess" in config and config["onlyPreprocess"]:
    ONLY_PREPROCESS = True

#################################################################
# CRAWLING
CRAWLTARGET = ""
TLD_CRAWL = ""
USERAGENT = ""
CRAWLSIZELIMIT = ""
CRAWLTIMELIMIT = ""
CRAWLWAIT = ""
CRAWLPAGELIMIT = ""
CRAWLFILETYPES = ""
CRAWLJOBS = "-j 2"
CRAWLTIMEOUT = ""
CRAWLDUMPARGS = ""
CONTINUECRAWL = ""
HERITRIXPATH = ""
HERITRIXURL = "https://localhost:8443"
HERITRIXUSER = "admin:admin"

if "crawler" in config:
    CRAWLTARGET = config["crawler"]

if "crawl-tld" in config and config["crawl-tld"]:
    TLD_CRAWL = "-D"

if "crawlerUserAgent" in config:
    USERAGENT = f'-a "{config["crawlerUserAgent"]}"'

if "crawlSizeLimit" in config:
    CRAWLSIZELIMIT = f'-s {config["crawlSizeLimit"]}'

if "crawlTimeLimit" in config:
    if CRAWLTARGET == "heritrix":
        CRAWLTIMELIMIT = config["crawlTimeLimit"]
    else:
        CRAWLTIMELIMIT = f'-t {config["crawlTimeLimit"]}'

if "crawlWait" in config:
    CRAWLWAIT = f'--wait {config["crawlWait"]}'

if "crawlFileTypes" in config:
    CRAWLFILETYPES = f'-f {config["crawlFileTypes"]}'

if "crawlerNumThreads" in config:
    CRAWLJOBS = f'-j {config["crawlerNumThreads"]}'

if "crawlerConnectionTimeout" in config:
    CRAWLTIMEOUT = f'-o {config["crawlerConnectionTimeout"]}'

if "dumpCurrentCrawl" in config:
    CRAWLDUMPARGS = f'-d {config["dumpCurrentCrawl"]}'

if "resumePreviousCrawl" in config:
    CONTINUECRAWL = f'-l {config["resumePreviousCrawl"]}'

if "heritrixPath" in config:
    HERITRIXPATH = config["heritrixPath"]

if "heritrixUrl" in config:
    HERITRIXURL = config["heritrixUrl"]

if "heritrixUser" in config:
    HERITRIXUSER = config["heritrixUser"]

#################################################################
# PREPROCESS
PPROC = "w2p"
GIAWARC = "~/go/bin/giawarc"
PPROC_FILES = ["plain_text.gz", "url.gz", "mime.gz", "normalized_html.gz", "deboilerplate_html.gz"]
if "preprocessor" in config and config["preprocessor"] == "giawarc":
    PPROC = "giawarc"
    PPROC_FILES = ["plain_text.gz", "url.gz", "mime.gz"]
    if "giawarc_executable" in config:
        GIAWARC = config["giawarc_executable"]

SHARDS = 8
BATCHES = 100
if "shards" in config:
    SHARDS = config["shards"]
if "batches" in config:
    BATCHES = config["batches"]

CLEANHTML = ""
FTFY = ""
LANGID = "cld2"
PARSER = ""
BOILERPIPE = ""
PDFEXTRACT = ""

if "cleanHTML" in config and config["cleanHTML"]:
    CLEANHTML = "--cleanhtml"
if "ftfy" in config and config["ftfy"]:
    FTFY = "--ftfy"
if "langID" in config:
    LANGID = config['langID']
if "parser" in config:
    PARSER = f"--parser {config['parser']}"
if "boilerpipeCleaning" in config and config["boilerpipeCleaning"]==True:
    BOILERPIPE = "--boilerpipe"
if "PDFextract" in config and config["PDFextract"]:
    PDFEXTRACT = "--pdfextract"

SENTTOKS = {} 
CUSTOMNBPS = {}
WORDTOKS = {}
MORPHTOKS = {}

if "sentenceSplitters" in config:
    SENTTOKS = config["sentenceSplitters"]
if "customNBPs" in config:
    CUSTOMNBPS = config["customNBPs"] 
if "wordTokenizers" in config:
    WORDTOKS = config["workTokenizers"]
if "morphologicalAnalysers" in config:
    MORPHTOKS = config["morphologicalAnalysers"]

# sentence splitting and tokenisation
PRUNE_THRESHOLD = "--prune 80"
PRUNE_TYPE = "--prune-type words"

if "pruneThreshold" in config:
    PRUNE_THRESHOLD = f"--prune {config['pruneThreshold']}"
if "pruneType" in config:
    PRUNE_TYPE = f"--prune-type {config['pruneType']}"

#################################################################
# DOCALIGN
DOCALIGN = 'dic'
if 'documentAligner' in config:
    DOCALIGN = config["documentAligner"]
# mt
MT_COMMAND = config['alignerCmd']
DOC_THRESHOLD = 0.1
DOCALIGN_THREADS = 1 
if "documentAlignerWorkers" in config:
    DOCALIGN_THREADS = config['documentAlignerWorkers']
if "documentAlignerThreshold" in config:
    DOC_THRESHOLD = config["documentAlignerThreshold"]
# dic
# TODO
#################################################################
# SEGALIGN
SEGALIGN = 'hunalign'
if "segmentAligner" in config:
    SEGALIGN = config["hunalign"]
SEGALIGN_THREADS = 1
if "sentenceAlignerWorkers" in config:
    SEGALIGN_THREADS = config["sentenceAlignerWorkers"]
# bleualign
BLEU_TRESHOLD = 0.1
if "sentenceAlignerThreshold" in config:
    BLEU_THRESHOLD=config["sentenceAlignerThreshold"]
# hunalign
# TODO
#################################################################
# CLEANING
FIELDS = ['url1','url2','seg1','seg2','aligner']
DEFERRED = False
DEFERRED_FIELDS = []
BIFIXER = False
BIFIXER_FIELDS = []
AGGRESSIVE_DEDUP = ""
BICLEANER = False
BICLEANER_MODEL = ""
BICLEANER_FIELDS = []
BICLEANER_THRESHOLD = 0.0
ELRC = False
ELRC_FIELDS = []
TMX = False
DEDUPED = False
# TODO: add rawCorpus option to generate lang1-lang2.raw.gz
OUTPUT_FILES = ["sent", "raw"]

if 'deferredCrawling' in config and config['deferredCrawling']:
    DEFERRED = True
    DEFERRED_FIELDS = ['deferredseg1','checksum1','deferredseg2','checksum2']
if 'bifixer' in config and config['bifixer']:
    BIFIXER = True
    BIFIXER_FIELDS = ['bifixerhash','bifixerscore']
if 'aggressiveDedup' in config and config['aggressiveDedup']:
    AGGRESSIVE_DEDUP = '--aggressive_dedup'
if 'bicleaner' in config:
    BICLEANER = True
    BICLEANER_MODEL = config['bicleaner']
    BICLEANER_FIELDS = ['bicleaner']
if 'bicleanerThreshold' in config:
    BICLEANER_THRESHOLD = config['bicleanerThreshold']
if 'elrc' in config and config['elrc']:
    ELRC = True
    ELRC_FIELDS = ['lengthratio','numTokensSL','numTokensTL']
if 'tmx' in config and config['tmx']:
    TMX = True
    OUTPUT_FILES.append('not-deduped.tmx')
if 'deduped' in config and config['deduped']:
    OUTPUT_FILES.append('deduped.tmx')
    OUTPUT_FILES.append('deduped.txt')

BEFORE_ELRC_FIELDS = FIELDS + DEFERRED_FIELDS + BIFIXER_FIELDS + BICLEANER_FIELDS
TMX_FIELDS = BEFORE_ELRC_FIELDS + ELRC_FIELDS

BIFIXER_HASH_COLUMN = ''
BIFIXER_SCORE_COLUMN = ''
BICLEANER_CACHE_DEDUP = "3,4"
BICLEANER_SORT = f"LC_ALL=C sort -t $'\t' -k3,4 -T {TMPDIR} --compress-program=gzip |"
DEDUP = 'seg1,seg2'
if 'bifixerhash' in BEFORE_ELRC_FIELDS:
    i = BEFORE_ELRC_FIELDS.index('bifixerhash')
    BIFIXER_HASH_COLUMN = f'{i},{i}'
    BIFIXER_SCORE_COLUMN = f'{i+1},{i+1}'
    BICLEANER_CACHE_DEDUP = f'{i}'
    BICLEANER_SORT = ""
    DEDUP = 'bifixerhash'

BEFORE_ELRC_FIELDS = ','.join(BEFORE_ELRC_FIELDS)
TMX_FIELDS = ','.join(TMX_FIELDS)
#################################################################
# DATASOURCES
HOSTS = set()
WARCS = set()

if "warcs" in config:
    WARCS = WARCS.union(config["warcs"])

if "hosts" in config:
    HOSTS = HOSTS.union(config["hosts"])

if "hostsFile" in config:
    with open_xz_or_gzip_or_plain(config["hostsFile"]) as f:
        for line in f:
            HOSTS.add(line.strip())

DOMAIN_2_HOSTS = create_domain_key_2_host_map(HOSTS)
# group together the WARCS that are in the same folder (process them individually, or all together?)
TARGET_2_WARCS = parent_folder_2_warcs(WARCS)
# group crawled hosts by domains
TARGET_2_WARCS.update(dict([(domain, [f'{DATADIR}/warc/{host}/{CRAWLTARGET}.warc.gz' for host in hosts]) for (domain, hosts) in DOMAIN_2_HOSTS.items()]))
TARGETS = TARGET_2_WARCS.keys()
#################################################################
OUTPUT = []

if ONLY_CRAWL:
    for domain, hosts in DOMAIN_2_HOSTS:
        for host in hosts:
            OUTPUT.append('{DATADIR}/warc/{host}/{CRAWLTARGET}.warc.gz')
elif ONLY_PREPROCESS:
    # OUTPUT = expand('{datadir}/preprocess/{domain}/{pproc}/{lang}/{pproc_file}', datadir=DATADIR, domain=TARGET_2_WARCS, pproc=PPROC, lang=LANGS, pproc_file=PPROC_FILES+["plain_tokenized.gz", "plain_sentences.gz"])
    OUTPUT = expand('{datadir}/preprocess/03.split.{lang}', datadir=DATADIR, lang=LANGS)
else:
    # OUTPUT = expand('{permanent}/{lang1}-{lang2}.{output_file}.xz', permanent=PERMANENT, target=TARGETS, lang1=LANG1, lang2=LANG2, output_file=OUTPUT_FILES)
    OUTPUT.append(f'{TRANSIENT}/06.1.segalign.fr_en')

shell.prefix("set -euo pipefail;")
rule all:
    input: OUTPUT

#################################################################
### CRAWLING ####################################################
rule creepy_download:
    params: url="http://{target}", folder=f"{DATADIR}/warc/{{target}}"
    output: f'{DATADIR}/warc/{{target}}/creepy.warc.gz'
    shell: '''
        mkdir -p {params.folder} {TMPDIR}
        python3 {BITEXTOR}/bitextor-creepy.py {TLD_CRAWL} {CRAWLSIZELIMIT} {CRAWLTIMELIMIT} {CRAWLWAIT} {CRAWLJOBS} {CRAWLTIMEOUT} {CRAWLDUMPARGS} {CONTINUECRAWL} {USERAGENT} {params.url} > {output}
        '''

rule httrack_download:
    params: url="http://{target}", folder=f"{DATADIR}/warc/{{target}}"
    output: f'{DATADIR}/warc/{{target}}/httrack.warc.gz'
    shell: '''
        mkdir -p {params.folder} {TMPDIR}
        echo hostname=$HOSTNAME
        DIRNAME=$(mktemp -d {TMPDIR}/downloaded.{wildcards.target}.XXXXXX)
        {BITEXTOR}/bitextor-httrack.py --url {params.url} --output-path $DIRNAME {CRAWLTIMELIMIT} {CRAWLPAGELIMIT} {USERAGENT} {CRAWLWAIT}
        {BITEXTOR}/bitextor-webdir2warc.sh $DIRNAME > {output}
        rm -rf $DIRNAME
        '''

rule wget_download:
    params: url="http://{target}", folder=f"{DATADIR}/warc/{{target}}"
    output: f'{DATADIR}/warc/{{target}}/wget.warc.gz'
    shell: '''
        mkdir -p {params.folder} {TMPDIR}
        echo hostname=$HOSTNAME
        DIRNAME=$(mktemp -d "{TMPDIR}/downloaded.{wildcards.target}.XXXXXX")
        {BITEXTOR}/bitextor-wget.py --url {params.url} --output-path $DIRNAME {CRAWLTIMELIMIT} {USERAGENT} {CRAWLFILETYPES} {CRAWLWAIT} --warc {output}
        rm -rf $DIRNAME
        '''

rule heritrix_download:
    params: url="http://{target}", folder=f"{DATADIR}/warc/{{target}}"
    output: f'{DATADIR}/warc/{{target}}/heritrix.warc.gz'
    shell: '''
        mkdir -p {params.folder} {TMPDIR}
        echo hostname=$HOSTNAME
        if [ "$(ps aux | grep -i Heritrix | grep -v grep)" == "" ] 
            then {HERITRIXPATH}/bin/heritrix -a {HERITRIXUSER}
        fi
        curl -v -d "action=teardown" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
        curl -v -d "createpath={wildcards.target}&action=create" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine
        DIRNAME=$(mktemp -d "{TMPDIR}/downloaded.{wildcards.target}.XXXXXX")
        cat {BITEXTOR}/crawler-beans.cxml | sed "s@http://example.example/example@{params.url}@g" > $DIRNAME/my-crawler-beans.cxml
        curl -v -T $DIRNAME/my-crawler-beans.cxml -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}/jobdir/crawler-beans.cxml
        curl -v -d "action=build" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
        curl -v -d "action=launch&checkpoint=latest" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
        sleep 2
        curl -v -d "action=unpause" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
        RUNTIME=0
        sleep 15
        while [ -f {HERITRIXPATH}/jobs/{wildcards.target}/latest/warcs/*warc.gz.open ]
        do
            sleep 5
            RUNTIME=$((RUNTIME+5))
            if [ "{CRAWLTIMELIMIT}" != "" ]
            then
                if [ $RUNTIME -gt "{CRAWLTIMELIMIT}" ] 
                then
                    echo "Crawling time limit reached"
                    curl -v -d "action=pause" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
                    curl -v -d "action=checkpoint" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
                    curl -v -d "action=terminate" -k -u {HERITRIXUSER} --anyauth --location {HERITRIXURL}/engine/job/{wildcards.target}
                fi
            fi
        done
        echo "Job {wildcards.target} finished!"
        cat {HERITRIXPATH}/jobs/{wildcards.target}/*/warcs/*warc.gz > {output}
    '''
#################################################################
### PREPROCESS ##################################################
pproc_output = {}
for pproc_file in PPROC_FILES:
    name = pproc_file.split('.')[0]
    for lang in LANGS:
        pproc_output[f"{lang}_{name}"] = f"{DATADIR}/preprocess/{{target}}/{PPROC}/{lang}/{pproc_file}"

# rule warc2preprocess:
#     input: lambda wildcards: TARGET_2_WARCS[wildcards.target]
#     output: **pproc_output
#     threads: 2
#     params: folder=f'{DATADIR}/preprocess/{{target}}/w2p', pproclangs=",".join(LANGS)
#     shell: '''
#         mkdir -p {params.folder}
#         cat {input} | {BITEXTOR}/bitextor-warc2htmlwarc.py {CLEANHTML} {FTFY} {PDFEXTRACT} --disable-output-gzip | {BITEXTOR}/bitextor-warc2preprocess.py --input - --langs {params.pproclangs} --compression gz --langid {LANGID} {BOILERPIPE} {PARSER} --output-dir {params.folder}
#         for lang in {LANGS}; do
#             if [ ! -f {params.folder}/$lang/plain_text.gz ]; then
#                 >&2 echo "WARNING: no \'$lang\' data found in {wildcards.target}. Creating empty files instead"
#                 mkdir -p {params.folder}/$lang
#                 touch {params.folder}/$lang/{{plain_text,mime,url,normalized_html,deboilerplate_html}}
#                 gzip {params.folder}/$lang/{{plain_text,mime,url,normalized_html,deboilerplate_html}}
#             fi
#         done
#     '''

rule giawarc:
    input: lambda wildcards: TARGET_2_WARCS[wildcards.target]
    output: **pproc_output
    params: folder=f'{DATADIR}/preprocess/{{target}}/giawarc'
    threads: 2
    shell: '''
        mkdir -p {params.folder}
        cat {input} | {BITEXTOR}/bitextor-warc2htmlwarc.py {CLEANHTML} {FTFY} {PDFEXTRACT} | ~/go/bin/giawarc -f bilang -l {LANGID} -o {params.folder} -
        for lang in {LANGS}; do
            if [ ! {params.folder}/$lang/plain_text.gz ]; then
                >&2 echo "WARNING: no \'$lang\' data found in {wildcards.target}. Creating empty files instead"
                mkdir -p {params.folder}/$lang
                touch {params.folder}/$lang/{{plain_text,mime,url}}
                gzip {params.folder}/$lang/{{plain_text,mime,url}}
            fi
        done
    '''

# DAG will be re-evaluated after completing shard rule (because number of batches is dynamic and unknown)
checkpoint shard:
    # use url.gz as input to avoid having directories as input
    input: expand("{datadir}/preprocess/{target}/{pproc}/{{lang}}/url.gz", datadir=DATADIR, target=TARGETS, pproc=PPROC)
    output: f'{DATADIR}/preprocess/02.batches.{{lang}}' # list of batches created for lang
    params:
        n = SHARDS,
        b = BATCHES,
        o = f'{DATADIR}/preprocess/shards/{{lang}}'
    shell: '''
        IFS=" " read -a input <<< "{input}"
        ulimit -n 2048
        giashard -n {params.n} -b {params.b} -o {params.o} ${{input[@]%/*}}
        ls -d {params.o}/*/* > {output}
        '''

# obtain list of batches for lang
def get_batches(lang):
    batches = []
    with checkpoints.shard.get(lang=lang).output[0].open() as f:
        for line in f:
            batches.append(line.strip())
    return batches

rule split:
    input: f'{DATADIR}/preprocess/shards/{{lang}}/{{shard}}/{{batch}}/plain_text.gz'
    params:
        splitter = lambda wildcards: get_lang_or_default(SENTTOKS, wildcards.lang),
        customnbp = lambda wildcards: get_customnbp(CUSTOMNBPS, wildcards.lang),
    output: f'{DATADIR}/preprocess/shards/{{lang}}/{{shard}}/{{batch}}/sentences.gz'
    shell: '''
        {BITEXTOR}/bitextor-split.py --text {input} \
                --sentence-splitter "{params.splitter}" \
                --langcode "{wildcards.lang}" --customnbp "{params.customnbp}" \
                {PRUNE_THRESHOLD} {PRUNE_TYPE} \
            | gzip -c > {output}
        '''

rule aggregate_split:
    input: lambda wildcards: [f'{batch}/sentences.gz' for batch in get_batches(wildcards.lang)]
    output: f'{DATADIR}/preprocess/03.split.{{lang}}'
    shell: ''' echo "{input}" | tr ' ' '\n' > {output} '''

#################################################################
### DOCALIGN ####################################################
def get_align_inputs(src_lang, trg_lang):
    src_batches = get_batches(src_lang)
    trg_batches = get_batches(trg_lang)
    # each input -> (shard, (src_batch, trg_batch))
    inputs = get_mt_docalign_inputs(src_batches, trg_batches)
    return inputs

rule aggregate_matches:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{src_batch}_{trg_batch}.06_01.matches' for (shard, (src_batch, trg_batch)) in get_align_inputs(LANG1, LANG2)]
    output: f'{TRANSIENT}/06_01.docalign.{LANG1}_{LANG2}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''
# MT ############################################################
rule custom_translate:
    input:
        source=f'{DATADIR}/preprocess/shards/{LANG1}/{{shard}}/{{src_batch}}/sentences.gz'
    output: f'{DATADIR}/preprocess/shards/{LANG1}/{{shard}}/{{src_batch}}/sentences_{LANG2}.gz' # TODO: put this in transient?
    shell: '''
        zcat {input.source} \
            | ~/go/bin/b64filter {BITEXTOR}/preprocess/bin/cache {MT_COMMAND} \
            | gzip -c > {output}
        n_before=$(zcat {input.source} | wc -l)
        n_after=$(zcat {output} | wc -l)
        echo "Check count $n_before -> $n_after for {LANG1}/{wildcards.shard}/{wildcards.src_batch}"
        '''
        # TODO: exit if counts are different?

rule aggregate_translate:
    input: lambda wildcards: [f'{batch}/sentences_{LANG2}.gz' for batch in get_batches(LANG2)]
    output: f'{TRANSIENT}/04.translate.{LANG2}'
    shell: ''' echo "{input}" | tr ' ' '\n' > {output} '''

rule tokenise_translated:
    input: rules.custom_translate.output
    output: f'{DATADIR}/preprocess/shards/{LANG1}/{{shard}}/{{src_batch}}/tokenised_{LANG2}.gz' # TODO: put this in transient?
        # temp(f"{TRANSIENT}/{{target}}/docalign/{LANG1}.customMT.extracted.translated.tokenized")
    params:
        tokeniser = lambda wildcards: get_lang_or_default(WORDTOKS, LANG2),
        lemmatizer = lambda wildcards: get_lang_or_default(MORPHTOKS, LANG2)
    shell: '''
        {BITEXTOR}/bitextor-tokenize.py --text {input} \
                --word-tokenizer "{params.tokeniser}" --morph-analyser "{params.lemmatizer}" \
                --langcode {LANG2} \
            | gzip -c > {output}
        '''

rule tokenise_target:
    input: f'{DATADIR}/preprocess/shards/{LANG2}/{{shard}}/{{trg_batch}}/sentences.gz'
    output: f'{DATADIR}/preprocess/shards/{LANG2}/{{shard}}/{{trg_batch}}/tokenised.gz'
    params:
        tokeniser = lambda wildcards: get_lang_or_default(WORDTOKS, LANG2),
        lemmatizer = lambda wildcards: get_lang_or_default(MORPHTOKS, LANG2)
    shell: '''
        {BITEXTOR}/bitextor-tokenize.py --text {input} \
                --word-tokenizer "{params.tokeniser}" --morph-analyser "{params.lemmatizer}" \
                --langcode {LANG2} \
            | gzip -c > {output}
        '''

rule aggregate_tokenise_translated:
    input: lambda wildcards: [f'{batch}/tokenised_{LANG2}.gz' for batch in get_batches(LANG1)]
    output: f'{TRANSIENT}/05.tokenise.{LANG1}_{LANG2}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''

rule aggregate_tokenise_target:
    input: lambda wildcards: [f'{batch}/tokenised.gz' for batch in get_batches(LANG2)]
    output: f'{TRANSIENT}/05.tokenise.{LANG2}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''

rule mt_matches:
    input:
        l1=rules.tokenise_translated.output,
        l2=rules.tokenise_target.output
    output: f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}/{{src_batch}}_{{trg_batch}}.06_01.matches'
    params: folder=f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}'
    threads: DOCALIGN_THREADS
    shell: "mkdir -p {params.folder}; {BITEXTOR}/document-aligner/bin/docalign {input.l1} {input.l2} --threshold {DOC_THRESHOLD} -j {DOCALIGN_THREADS} > {output}"
# DIC ###########################################################
# TODO
#################################################################
### SEGALIGN ####################################################
rule aggregate_segalign:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{src_batch}_{trg_batch}.06_02.segalign.gz' for (shard, (src_batch, trg_batch)) in get_align_inputs(LANG1, LANG2)]
    output: f'{TRANSIENT}/06_02.segalign.{LANG1}_{LANG2}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''
# BLEUALIGN #####################################################
rule bleualign:
    input:
        indices=rules.mt_matches.output,
        plain1=f'{DATADIR}/preprocess/shards/{LANG1}/{{shard}}/{{src_batch}}/sentences.gz',
        plain2=f'{DATADIR}/preprocess/shards/{LANG2}/{{shard}}/{{trg_batch}}/sentences.gz',
        url1=f'{DATADIR}/preprocess/shards/{LANG1}/{{shard}}/{{src_batch}}/url.gz',
        url2=f'{DATADIR}/preprocess/shards/{LANG2}/{{shard}}/{{trg_batch}}/url.gz',
        translated1=rules.custom_translate.output
    params: folder=f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}'
    output:
        f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}/{{src_batch}}_{{trg_batch}}.06_02.segalign.gz'
    threads: max(SEGALIGN_THREADS, 2) 
    shell: '''
        mkdir -p {params.folder}
        parallel_cmd=""
        if [ {SEGALIGN_THREADS} -gt 1 ]; then
            parallel_cmd="parallel --gnu --halt 2 --pipe --j {SEGALIGN_THREADS} --line-buffer"
        fi
        cat {input.indices} \
            | {BITEXTOR}/document-aligner/bin/docjoin \
                -l {input.url1} -r {input.url2} \
                -l {input.plain1} -r {input.plain2} \
                -l {input.translated1} \
            | ${{parallel_cmd}} {BITEXTOR}/bleualign-cpp/bleualign_cpp --bleu-threshold {BLEU_TRESHOLD} \
            | gzip -c > {output}
        '''
# HUNALIGN ######################################################
# TODO
#################################################################
### FILTERING AND CLEANING ######################################

# TODO: deferred_documents does not work with giawarc: html of the original document not saved
# rule deferred_documents:
#     input:
#         html=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/normalized_html.gz',
#         url=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/url.gz'
#     output:
#         text=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/html5lib_plain_text.xz',
#         deferred=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/deferred_documents.xz'
#     shell: '''
#         touch {output.text}.touch && xz {output.text}.touch && mv {output.text}.touch.xz {output.text}
#         touch {output.deferred}.touch && xz {output.deferred}.touch && mv {output.deferred}.touch.xz {output.deferred}
#         paste <(zcat {input.html}) <(zcat {input.url}) \
#             | python3 {BITEXTOR}/standoff/deferred-documents.py \
#             | awk '{{ print $1 | "xz > {output.text}"; print $3 | "xz > {output.deferred}" }}'
#         '''

# deferred_input = rules.bleualign.output
# if SEGALIGN == "hunalign":
#     deferred_input = rules.hunalign.output

# rule deferred_segments:
#     input:
#         deferred_input,
#         f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG1}/html5lib_plain_text.xz',
#         f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG1}/url.gz',
#         f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG1}/deferred_documents.xz',
#         f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/html5lib_plain_text.xz',
#         f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/url.gz',
#         f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/deferred_documents.xz'
#     output: temp(f'{TRANSIENT}/{{target}}/deferred')
#     shell: '''
#         xzcat -T 0 -f {input[0]} \
#             | python3 {BITEXTOR}/standoff/deferred-sentences.py <(paste <(xzcat {input[1]} {input[4]}) <(zcat {input[2]} {input[5]}) <(xzcat {input[3]} {input[6]})) \
#             > {output}
#         '''

# bifixer_input = rules.deferred_segments.output
# if not DEFERRED:
#     bifixer_input = rules.deferred_segments.input[0]
# bifixer_input = deferred_input

rule bifixer:
    input: rules.bleualign.output
    output: temp(f'{TRANSIENT}/{LANG1}-{LANG2}/{{shard}}/{{src_batch}}_{{trg_batch}}.07_01.bifixer')
    shell: '''
        zcat {input} \
            | python3 {BITEXTOR}/bifixer/bifixer/bifixer.py -q - - {LANG1} {LANG2} {AGGRESSIVE_DEDUP} \
            | LC_ALL=C sort -t $'\t' -k{BIFIXER_HASH_COLUMN} -k{BIFIXER_SCORE_COLUMN}nr -T {TMPDIR} --compress-program=gzip -n -r \
            > {output}
        '''

bicleaner_input = rules.bifixer.output
if not BIFIXER:
    bicleaner_input = rules.bifixer.input

rule bicleaner:
    input: bifixer=bicleaner_input, model=BICLEANER_MODEL
    output: temp(f'{TRANSIENT}/{LANG1}-{LANG2}/{{shard}}/{{src_batch}}_{{trg_batch}}.07_02.bicleaner')
    threads: 2
    shell: '''
        CAT=cat; if [[ {input.bifixer} == *.gz ]]; then CAT=zcat; fi
        slang=$(egrep "source_lang" {input.model} | cut -d " " -f 2)
        if [ "$slang" == "{LANG1}" ]; then
            $CAT {input.bifixer} \
                | {BITEXTOR}/preprocess/bin/cache -k {BICLEANER_CACHE_DEDUP} python3 {BITEXTOR}/bicleaner/bicleaner/bicleaner_classifier_lite.py --score-only -q - - {input.model} \
                | paste <(cat {input.bifixer}) - \
                > {output}
        else
            $CAT {input.bifixer} \
                | awk ' BEGIN {{FS="\t"; OFS="\t"}} {{ t = $3; $3 = $4; $4 = t; print;}} ' \
                | {BITEXTOR}/preprocess/bin/cache -k {BICLEANER_CACHE_DEDUP} python3 {BITEXTOR}/bicleaner/bicleaner/bicleaner_classifier_lite.py --score-only -q - - {input.model} \
                | paste <(cat {input.bifixer}) - \
                > {output}
        fi
        '''

filter_input = rules.bicleaner.output
if not BICLEANER:
    filter_input = rules.bicleaner.input

rule filter:
    input: filter_input
    output: temp(f'{TRANSIENT}/{LANG1}-{LANG2}/{{shard}}/{{src_batch}}_{{trg_batch}}.07_03.filtered')
    threads: 2
    shell: '''
        CAT=cat; if [[ {input} == *.gz ]]; then CAT=zcat; fi
        if [[ {BICLEANER} -eq "True" ]] && [[ {ELRC} -eq "True" ]]; then
            $CAT {input} \
                | python3 {BITEXTOR}/bitextor-filterbicleaner.py --threshold {BICLEANER_THRESHOLD} \
                | python3 {BITEXTOR}/bitextor-elrc-filtering.py -c "{BEFORE_ELRC_FIELDS}" -s \
                > {output}
        elif [[ {BICLEANER} -eq "True" ]] && [[ {ELRC} -eq "False" ]]; then
            $CAT {input} \
                | python3 {BITEXTOR}/bitextor-filterbicleaner.py --threshold {BICLEANER_THRESHOLD} \
                > {output}
        elif [[ {BICLEANER} -eq "False" ]] && [[ {ELRC} -eq "True" ]]; then
            $CAT {input} \
                | {BITEXTOR}/bitextor-elrc-filtering.py -c "{BEFORE_ELRC_FIELDS}" -s \
                > {output}
        fi
        '''

raw_input = rules.bicleaner.output
if not BICLEANER:
    raw_input = rules.bicleaner.input
raw_input_filename = '.'.join(raw_input[0].split('.')[-2:]) # 06_02.segalign.gz / 07_01.bifixer / 07_02.bicleaner

sents_input = rules.filter.output
if not ELRC and not BICLEANER:
    sents_input = rules.filter.input
sents_input_filename = '.'.join(raw_input[0].split('.')[-2:]) # 06_02.segalign.gz / 07_01.bifixer / 07_02.bicleaner / 07_03.filtered

rule raw:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{src_batch}_{trg_batch}.{raw_input_filename}' for (shard, (src_batch, trg_batch)) in get_align_inputs(LANG1, LANG2)]
    output: f'{TRANSIENT}/{LANG1}-{LANG2}.raw.gz'
    shell: ''' 
        if [[ {input[0]} == *.gz ]]; then
            cat {input} > {output}
        else
            cat {input} | gzip -c > {output}
        fi
        '''

rule sents:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{src_batch}_{trg_batch}.{sents_input_filename}' for (shard, (src_batch, trg_batch)) in get_align_inputs(LANG1, LANG2)]
    output: f'{PERMANENT}/{LANG1}-{LANG2}.sent.gz'
    shell: '''
        if [[ {input[0]} == *.gz ]]; then
            cat {input} > {output}
        else 
            cat {input} | gzip -c > {output}
        fi
        '''

rule tmx:
    input: rules.sents.output
    output: f'{PERMANENT}/{LANG1}-{LANG2}.not-deduped.tmx.xz'
    shell: '''
        xzcat -T 0 -f {input} \
            | python3 {BITEXTOR}/bitextor-buildTMX.py --lang1 {LANG1} --lang2 {LANG2} -c {TMX_FIELDS} \
            | xz -T 0 -c > {output}
        '''

rule deduped_tmx:
    input: rules.sents.output
    output:
        tmx=f'{PERMANENT}/{LANG1}-{LANG2}.deduped.tmx.xz',
        txt=f'{PERMANENT}/{LANG1}-{LANG2}.deduped.txt.xz'
    shell: '''
        xzcat -T 0 -f {input} \
            | {BICLEANER_SORT} {BITEXTOR}/bitextor-buildTMX.py --lang1 {LANG1} --lang2 {LANG2} -c {TMX_FIELDS} --dedup "{DEDUP}" -f {output.txt} \
            | xz -T 0 -c > {output.tmx}
        '''
