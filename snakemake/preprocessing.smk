# required options
BITEXTOR = config["bitextor"]
DATADIR = config["dataDir"]
LANGS = config["langs"]
WARCS = config["warcs"]

# preprocessing
PPROC = "w2p"
GIAWARC = "~/go/bin/giawarc"
FILES = ["plain_text.gz", "url.gz", "mime.gz", "normalized_html.gz", "deboilerplate_html.gz"]
if "preprocessor" in config and config["preprocessor"] == "giawarc":
	PPROC = "giawarc"
	FILES = ["plain_text.gz", "url.gz", "mime.gz"]
	if "giawarc_executable" in config:
		GIAWARC = config["giawarc_executable"]
CLEANHTML = ""
FTFY = ""
LANGID = "cld2"
PARSER = ""
BOILERPIPE = ""
PDFEXTRACT = ""

# sentence splitting and tokenisation
SENTTOKS = {} 
CUSTOMNBPS = {}
WORDTOKS = {}
MORPHTOKS = {}
PRUNE_THRESHOLD = "--prune 80"
PRUNE_TYPE = "--prune-type words"

if "sentenceSplitters" in config:
	SENTTOKS = config["sentenceSplitters"]
if "customNBPs" in config:
	CUSTOMNBPS = config["customNBPs"] 
if "wordTokenizers" in config:
	WORDTOKS = config["workTokenizers"]
if "morphologicalAnalysers" in config:
	MORPHTOKS = config["morphologicalAnalysers"]
if "pruneThreshold" in config:
	PRUNE_THRESHOLD = f"--prune {config['pruneThreshold']}"
if "pruneType" in config:
	PRUNE_TYPE = f"--prune-type {config['pruneType']}"


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

##################################################################

rule preprocess_all:
	input: expand("{datadir}/preprocess/{domain}/{pproc}/{lang}/{pproc_file}", datadir=DATADIR, domain=TARGET_2_WARCS, pproc=PPROC, lang=LANGS, pproc_file=FILES+["plain_tokenized.gz", "plain_sentences.gz"])

rule warc2preprocess:
	input: lambda wildcards: TARGET_2_WARCS[wildcards.target]
	output: expand("{datadir}/preprocess/{{target}}/w2p/{lang}/{pproc_file}", datadir=DATADIR, lang=LANGS, pproc_file=FILES)
	threads: 2
	params: folder=f'{DATADIR}/preprocess/{{target}}/w2p', pproclangs=",".join(LANGS)
	shell: '''
		mkdir -p {params.folder}
		cat {input} | {BITEXTOR}/bitextor-warc2htmlwarc.py {CLEANHTML} {FTFY} {PDFEXTRACT} --disable-output-gzip | {BITEXTOR}/bitextor-warc2preprocess.py --input - --langs {params.pproclangs} --compression gz --langid {LANGID} {BOILERPIPE} {PARSER} --output-dir {params.folder}
		for lang in {LANGS}; do
			if [ ! -f {params.folder}/$lang/plain_text.gz ]; then
				>&2 echo "WARNING: no \'$lang\' data found in {wildcards.target}. Creating empty files instead"
				mkdir -p {params.folder}/$lang
				touch {params.folder}/$lang/{{plain_text,mime,url,normalized_html,deboilerplate_html}}
				gzip {params.folder}/$lang/{{plain_text,mime,url,normalized_html,deboilerplate_html}}
			fi
		done
	'''

rule giawarc:
	input: lambda wildcards: TARGET_2_WARCS[wildcards.target]
	output: expand("{datadir}/preprocess/{{target}}/giawarc/{lang}/{pproc_file}", datadir=DATADIR, lang=LANGS, pproc_file=FILES)
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

rule tokenise:
	input: f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/plain_text.gz'
	params:
		splitter = lambda wildcards: get_lang_or_default(SENTTOKS, wildcards.lang),
		customnbp = lambda wildcards: get_customnbp(CUSTOMNBPS, wildcards.lang),
		tokeniser = lambda wildcards: get_lang_or_default(WORDTOKS, wildcards.lang),
		lemmatizer = lambda wildcards: get_lang_or_default(MORPHTOKS, wildcards.lang),
	output:
		tok = f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/plain_tokenized.gz',
		sent = f'{DATADIR}/preprocess/{{target}}/{PPROC}/{{lang}}/plain_sentences.gz'
	shell: '''
		{BITEXTOR}/bitextor-tokenize.py --text {input} --sentence-splitter "{params.splitter}" --word-tokenizer "{params.tokeniser}" --morph-analyser "{params.lemmatizer}" --langcode "{wildcards.lang}" --customnbp "{params.customnbp}" --sentences-output {output.sent} --tokenized-output {output.tok} {PRUNE_THRESHOLD} {PRUNE_TYPE}
		'''
