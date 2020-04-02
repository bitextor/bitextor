FIELDS = ['url1','url2','seg1','seg2','aligner']
DEFERRED = False
DEFERRED_FIELDS = []
BIFIXER = False
BIFIXER_FIELDS = []
AGGRESIVE_DEDUP = ""
BICLEANER = ""
BICLEANER_FIELDS = []
BICLEANER_THRESHOLD = 0.0
ELRC = False
ELRC_FIELDS = []
TMX = False
DEDUPED = False
FILES = ["sent", "raw"]

if 'deferredCrawling' in crawling and crawling['deferredCrawling']:
	DEFERRED = True
	DEFERRED_FIELDS = ['deferredseg1','checksum1','deferredseg2','checksum2']
if 'bifixer' in config and config['bifixer']:
	BIFIXER = True
	BIFIXER_FIELDS = ['bifixerhash','bifixerscore']
if 'aggresiveDedup' in config and config['aggresiveDedup']:
	AGGRESIVE_DEDUP = '--aggressive_dedup'
if 'bicleaner' in config:
	BICLEANER = config['bicleaner']
	BICLEANER_FIELDS = ['bicleaner']
if 'bicleanerThreshold' in config:
	BICLEANER_THRESHOLD = config['bicleanerThreshold']
if 'elrc' in config and config['elrc']:
	ELRC = True
	ELRC_FIELDS = ['lengthratio','numTokensSL','numTokensTL']

BEFORE_ELRC_FIELDS = FIELDS + DEFERRED_FIELDS + BIFIXER_FIELDS + BICLEANER_FIELDS
TMX_FIELDS = BEFORE_ELRC_FIELDS + ELRC_FIELDS
BIFIXER_HASH_COLUMN = BEFORE_ELRC_FIELDS.index('bifixerhash')
BIFIXER_SCORE_COLUMN = BIFIXER_HASH_COLUMN + 1


BEFORE_ELRC_FIELDS = ','.join(BEFORE_ELRC_FIELDS)
TMX_FIELDS = ','.join(TMX_FIELDS)

rule cleaning_all:
	input:
		expand("{transient}/{target}/segclean.xz", transient=TRANSIENT, target=TARGETS),
		expand("{permanent}/{target}/{lang1}-{lang2}.{file}.xz", permanent=PERMANENT, target=TARGETS, file=FILES)

rule clean:
	input: f'{TRANSIENT}/{{target}}/segalign.xz'
	output: f'{TRANSIENT}/{{target}}/segclean.xz'
	shell: '''

		'''
	
