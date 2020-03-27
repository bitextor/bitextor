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

###################################################################################
OUTPUT = []
for tld, hosts in DOMAIN_2_HOSTS.items():
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
