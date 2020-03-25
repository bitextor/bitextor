import sys
import os
import tldextract
from cerberus import Validator

def validate_args(config):
	schema = {
			'bitextor': {'required': True, 'type': 'string'},
			'hosts': {'type': 'list'},
			'hostsFile': {'type': 'string', 'check_with': os.path.isfile},
			'dataDir': {'type': 'string', 'required': True},
			'tempDir': {'type': 'string'},
			'crawler': {'required': True, 'type': 'string', 'allowed': ["wget", "heritrix", "creepy", "httrack"]},
			
			'crawlTimeLimit': {'type': 'string'},
			'crawlerUserAgent': {'type': 'string', 'dependencies': {'cralwer' : ['creepy', 'wget', 'httrack']}},
			'crawlWait': {'type': 'string', 'dependencies': {'crawler': ['creepy', 'wget', 'httrack']}},
			'crawlPageLimit': {'type': 'string', 'dependencies': {'crawler' : 'httrack'}},
			'crawlFileTypes': {'type': 'string', 'dependencies': {'crawler' : 'wget'}},
			'crawl-tld': {'type': 'boolean', 'dependencies': {'crawler' : 'creepy'}},
			'crawlSizeLimit': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
			'crawlerNumThreads': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
			'crawlerConnectionTimeout': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
			'dumpCurrentCrawl': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
			'resumePreviousCrawl': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
			'heritrixPath': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
			'heritrixUrl': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
			'heritrixUser': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}}
			}


	if 'crawler' in config and config['crawler'] == "heritrix":
		schema['heritrixPath']['required'] = True
	
	v = Validator(schema)
	v.allow_unknown = True
	b = v.validate(config)

	if not b:
		print("Validation error. Stopping.", v.errors, file=sys.stderr)
		exit()
			
###################################################################################
def create_domain_key_2_host_map(hosts):
	key2hosts = {}
	for host in hosts:
		# don't merge blog sites
		if host.find(".blogspot.") >= 0 or host.find(".wordpress.") >= 0:
			key = host
		else:
			key = tldextract.extract(host).domain
		
		if key not in key2hosts:
			key2hosts[key] = []
		key2hosts[key].append(host)
	return key2hosts
###################################################################################

validate_args(config)

sys.path.append(os.path.dirname(os.path.abspath(config["bitextor"]) + "/utils")) 
from utils.common import open_xz_or_gzip_or_plain

BITEXTOR = config["bitextor"]

DATADIR = config["dataDir"]

if "tempDir" in config:
	TMPDIR = config["tempDir"]
else:
	TMPDIR = "/tmp"

CRAWLTARGET = config["crawler"]

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


hosts = set()

if "hosts" in config:
	hosts = hosts.union(config["hosts"])

if "hostsFile" in config:
	with open_xz_or_gzip_or_plain(config["hostsFile"]) as f:
		for line in f:
			hosts.add(line.strip())

domain_key2hosts = create_domain_key_2_host_map(hosts)
###################################################################################
OUTPUT = []
for tld, hosts in domain_key2hosts.items():
	for host in hosts:
		OUTPUT.append(f'{DATADIR}/preprocess/{tld}/{host}.warc.gz')

rule crawling_all:
	# input: expand("{data}/warc/{target}/{crawler}.warc.gz", data=DATADIR, target=hosts, crawler=CRAWLTARGET)
	input: OUTPUT

# preprocessing step groups WARCs by parent folder, so symlinking from domain folder is enough
rule symlink_subdomains:
	input: f'{DATADIR}/warc/{{host}}/{CRAWLTARGET}.warc.gz'
	output: f'{DATADIR}/preprocess/{{target}}/{{host}}.warc.gz'
	shell: "ln -sfn {input} {output}"

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
