MT_COMMAND = config['alignerCmd']
DOC_THRESHOLD = 0.1
if "documentAlignerThreshold" in config:
	DOC_THRESHOLD = config["documentAlignerThreshold"]

WORDTOK2 = get_lang_or_default(WORDTOKS, LANG2)
MORPHTOK2 = get_lang_or_default(MORPHTOKS, LANG2)
if WORDTOK2 == "":
	get_default_tokeniser(BITEXTOR, LANG2)

rule mt_docalign_all:
	input: expand("{transient}/{target}/{lang1}-{lang2}.matches", transient=TRANSIENT, target=TARGETS, lang1=LANG1, lang2=LANG2)

rule sentences2extracted:
	input: f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG1}/plain_sentences.gz'
	output: temp(f'{TRANSIENT}/{{target}}/docalign/{LANG1}.extracted.xz')
	params: docalign_folder = f'{TRANSIENT}/{{target}}/docalign'
	shell: '''
		mkdir -p {params.docalign_folder}
		zcat {input} | {BITEXTOR}/document-aligner/utils/extract_lett.py | xz -T 0 -c > {output}
		'''

rule custom_translate:
	input: 
		source = rules.sentences2extracted.output,
		target = f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/plain_sentences.gz'
	output: temp(f'{TRANSIENT}/{{target}}/docalign/{LANG1}.customMT.extracted.translated.xz')
	shell: '''
		xzcat -T 0 -f {input.source} | cut -f 2 |
		{BITEXTOR}/preprocess/bin/cache {MT_COMMAND} |
		paste <(xzcat -T 0 -f {input.source} | cut -f 1) - |
		xz -c -T 0 -f > {output}
		'''

rule tokenize_translated:
	input: rules.custom_translate.output
	output: temp(f"{TRANSIENT}/{{target}}/docalign/{LANG1}.customMT.extracted.translated.tokenized")
	shell: '''
		if [-z "{MORPHTOK2}" ]; then
			xzcat -T 0 -f {input} | cut -f 2 |
			{WORDTOK2} | awk '{{print tolower($0)}}' |
			paste <(xzcat -T 0 -f {input} | cut -f 1) - |
			xz -T 0 -c -f > {output}
		else
			xzcat -T 0 -f {input} | cut -f 2 |
			{WORDTOK2} | {MORPHTOK2} | awk '{{print tolower($0)}}' |
			paste <(xzcat -T 0 -f {input} | cut -f 1) - |
			xz -T 0 -f > {output}
		'''

rule translated2base64:
	input: rules.custom_translate.output
	output: f'{TRANSIENT}/{{target}}/docalign/{LANG1}.translated_sentences.xz'
	shell: "xzcat -T 0 -f {input} | {BITEXTOR}/document-aligner/utils/extracted2base64.py | xz -T 0 -c > {output}"

rule translated_tokenized2base64:
	input: rules.tokenize_translated.output
	output: f'{TRANSIENT}/{{target}}/docalign/{LANG1}.translated_tokenized.xz'
	shell: "xzcat -T 0 -f {input} | {BITEXTOR}/document-aligner/utils/extracted2base64.py | xz -T 0 -c > {output}"

rule mt_matches:
	input: 
		l1=rules.tokenize_translated.output,
		l2=f'{DATADIR}/preprocess/{{target}}/{PPROC}/{LANG2}/plain_tokenized.gz'
	output: f'{TRANSIENT}/{{target}}/{LANG1}-{LANG2}.matches'
	shell:
		"python3 {BITEXTOR}/document-aligner/compute_matches.py --lang1 {input.l1} --lang2 {input.l2} --output-matches {output} --threshold {DOC_THRESHOLD}"
