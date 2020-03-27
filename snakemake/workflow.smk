import os
import sys

include: "utils.smk"

validate_args(config)

sys.path.append(os.path.dirname(os.path.abspath(config["bitextor"]) + "/utils"))
from utils.common import open_xz_or_gzip_or_plain

### BASIC PARAMETERS ###
BITEXTOR = config["bitextor"]
DATADIR = config["dataDir"]
TRANSIENT = config["transientDir"]
PERMANENT = config["permanentDir"]
TMPDIR = config["transientDir"]
if "tempDir" in config:
	TMPDIR = config["tempDir"]
### BASIC PREPROCESSING VARIABLES ###
PPROC = "w2p"
if "preprocessor" in config and config["preprocessor"] = "giawarc":
	PPROC = "giawarc"
### DEFINE DATASOURCES ###
HOSTS = set()

if "hosts" in config:
	hosts = hosts.union(config["hosts"])

if "hostsFile" in config:
	with open_xz_or_gzip_or_plain(config["hostsFile"]) as f:
		for line in f:
			hosts.add(line.strip())

DOMAIN_2_HOSTS = create_domain_key_2_hosts_map(hosts)
include: "crawling.smk"

# by manually manipulating the config it is possible to connect different workflows
if "warcs" not in config:
	config["warcs"] = []
config["warcs"].extend(rules.crawling_all.input)

TARGET_2_WARCS = parent_folder_2_warcs(config["warcs"])
TARGETS = TARGET_2_WARCS.keys()

include: "preprocessing.smk"

rule all:
	input: rules.preprocess_all.input
