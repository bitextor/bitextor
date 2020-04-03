import os
import sys

include: "utils.smk"

validate_args(config)

sys.path.append(os.path.dirname(os.path.abspath(config["bitextor"]) + "/utils"))
from utils.common import open_xz_or_gzip_or_plain

#################################################################
# BASIC PARAMETERS
# parameters that are exclusive to each snakefile are specified there
BITEXTOR = config["bitextor"]
DATADIR = config["dataDir"]
TRANSIENT = config["transientDir"]
PERMANENT = config["permanentDir"]
TMPDIR = config["transientDir"]
if "tempDir" in config:
	TMPDIR = config["tempDir"]

PPROC = "w2p"
if "preprocessor" in config and config["preprocessor"] == "giawarc":
	PPROC = "giawarc"

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
# DEFINE DATASOURCES
HOSTS = set()

if "hosts" in config:
	HOSTS = HOSTS.union(config["hosts"])

if "hostsFile" in config:
	with open_xz_or_gzip_or_plain(config["hostsFile"]) as f:
		for line in f:
			hosts.add(line.strip())

DOMAIN_2_HOSTS = create_domain_key_2_host_map(HOSTS)

# by manually manipulating the config it is possible to connect different workflows
include: "crawling.smk"
if "warcs" not in config:
	config["warcs"] = []
config["warcs"].extend(rules.crawling_all.input)

TARGET_2_WARCS = parent_folder_2_warcs(config["warcs"])
TARGETS = TARGET_2_WARCS.keys()
#################################################################
OUTPUT = []
if ONLY_CRAWL:
	OUTPUT = rules.crawling_all.input
elif ONLY_PREPROCESS:
	include: "preprocessing.smk"
	OUTPUT = rules.preprocess_all.input
else:
	include: "preprocessing.smk"
	include: "mt-docalign.smk"
	include: "bleualign.smk"
	include: "cleaning.smk"
	OUTPUT = rules.cleaning_all.input

rule all:
	input: OUTPUT
