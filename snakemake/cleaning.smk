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
# TODO: add rawCorpus option to generate lang1-lang2.raw.xz ((what is it supposed to be?))
FILES = ["sent"]

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
	FILES.append('not-deduped.tmx')
if 'deduped' in config and config['deduped']:
	FILES.append('deduped.tmx')
	FILES.append('deduped.txt')

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

rule cleaning_all:
	input: expand("{permanent}/{lang1}-{lang2}.{file}.xz", permanent=PERMANENT, target=TARGETS, lang1=LANG1, lang2=LANG2, file=FILES) 

# TODO: add deferred
rule bifixer:
	input: f'{TRANSIENT}/{{target}}/segalign.xz'
	output: temp(f'{TRANSIENT}/{{target}}/bifixer')
	shell: '''
		xzcat -T 0 -f {input} \
			| python3 {BITEXTOR}/bifixer/bifixer/bifixer.py -q - - {LANG1} {LANG2} {AGGRESSIVE_DEDUP} \
			| LC_ALL=C sort -t $'\t' -k{BIFIXER_HASH_COLUMN} -k{BIFIXER_SCORE_COLUMN}nr -T {TMPDIR} --compress-program=gzip -n -r \
			> {output}
		'''

bicleaner_input = rules.bifixer.output
if not BIFIXER:
	bicleaner_input = rules.bifixer.input

rule bicleaner:
	input: bifixer=bicleaner_input, model=BICLEANER_MODEL
	output: temp(f'{TRANSIENT}/{{target}}/bicleaner')
	threads: 2
	shell: '''
		CAT=cat; if [[ {input.bifixer} == *.xz ]]; then CAT=xzcat; fi
		slang=$(egrep "source_lang" {input.model} | cut -d " " -f 2)
		if [ "$slang" == "{LANG1}" ]; then
			$CAT {input.bifixer} \
				| {BITEXTOR}/preprocess/bin/cache -k {BICLEANER_CACHE_DEDUP} python3 {BITEXTOR}/bicleaner/bicleaner/bicleaner_classifier_lite.py --score-only -q - - {params.model} \
				| paste <(cat {input.bifixer}) - \
				| python3 {BITEXTOR}/bitextor-filterbicleaner.py --threshold {BICLEANER_THRESHOLD} \
				> {output}
		else
			$CAT {input.bifixer} \
				| awk ' BEGIN {{FS="\t"; OFS="\t"}} {{ t = $3; $3 = $4; $4 = t; print;}} ' \
				| {BITEXTOR}/preprocess/bin/cache -k {BICLEANER_CACHE_DEDUP} python3 {BITEXTOR}/bicleaner/bicleaner/bicleaner_classifier_lite.py --score-only -q - - {params.model} \
				| paste <(cat {input.bifixer}) - \
				| python3 {BITEXTOR}/bitextor-filterbicleaner.py --threshold {BICLEANER_THRESHOLD} \
				> {output}
		fi
		'''

elrc_input = rules.bicleaner.output
if not BICLEANER:
	elrc_input = rules.bicleaner.input

rule elrc:
	input: elrc_input
	output: temp(f'{TRANSIENT}/{{target}}/elrc')
	shell: '''
		CAT=cat; if [[ {input} == *.xz ]]; then CAT=xzcat; fi
		$CAT {input} \
			| {BITEXTOR}/bitextor/elrc/filtering.py -c "{BEFORE_ELRC_FIELDS}" -s \
			| xz -T 0 > {output}
		'''

sents_input = rules.elrc.output
if not ELRC:
	sents_input = rules.elrc.input
sents_input_filename = sents_input[0].split('/')[-1] # 'segaligz.xz'/'bifixer'/'bicleaner'/'elrc'

rule sents:
	input: expand("{transient}/{target}/{filename}", transient=TRANSIENT, target=TARGETS, filename=sents_input_filename)
	output: f'{PERMANENT}/{LANG1}-{LANG2}.sent.xz'
	shell: '''
		CAT=cat; if [[ {input[0]} == *.xz ]]; then CAT=xzcat; fi
		$CAT {input} | xz -T 0 -c > {output}
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
