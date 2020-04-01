BLEU_TRESHOLD = 0.1
if "sentenceAlignerThreshold" in config:
	BLEU_THRESHOLD=config["sentenceAlignerThreshold"]

rule bleualign_all:
	input: expand("{transient}/{target}/bleualign.xz", transient=TRANSIENT, target=TARGETS)

rule bleualign:
	input:
		indices=f'{TRANSIENT}/{{target}}/{LANG1}-{LANG2}.matches',
		plain1=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG1}/plain_sentences.gz',
		plain2=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/plain_sentences.gz',
		url1=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG1}/url.gz',
		url2=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/url.gz',
		translated1=f'{TRANSIENT}/{{target}}/docalign/{LANG1}.translated_sentences.xz'
	output:
		f'{TRANSIENT}/{{target}}/bleualign.xz'
	threads: 2
	shell: '''
		cut -f 2,3 {input.indices} | # assuming indices come from mt-docalign
		LC_ALL=C sort -nk1 | 
		python3 {BITEXTOR}/bitextor-build-docalign.py --columns1 {input.url1} {input.plain1} {input.translated1} --columns2 {input.url2} {input.plain2} |
		awk -F '\t' '{{print $2,$6,$3,$7,$4}} OFS='\t' |
		{BITEXTOR}/bleualign-cpp/bleualign_cpp --bleu-threhsold {BLEU_TRESHOLD} |
		xz -T 0 -c > {output}
		'''

