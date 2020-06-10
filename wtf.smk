import os
import sys
from itertools import product
#################################################################
# BASIC PARAMETERS
BITEXTOR = config["bitextor"]
# DATADIR = config["dataDir"]
# TRANSIENT = config["transientDir"]
# PERMANENT = config["permanentDir"]
# TMPDIR = config["transientDir"]
DATADIR = "/home/esarrias/test/wtf/data"
TRANSIENT = "/home/esarrias/test/wtf/transient"
PERMANENT = "/home/esarrias/test/wtf/permanent"
TMPDIR = "/home/esarrias/test/wtf/transient"

LANG1 = config["lang1"]
LANG2 = config["lang2"]
LANGS = [LANG1, LANG2]

PROFILING = "/usr/bin/time -v"
#################################################################
SRC_LANG = LANG1
TRG_LANG = LANG2
#################################################################
OUTPUT = []
OUTPUT_FILES = ['sent', 'not-deduped.tmx']
UNTIL = config['until'] if 'until' in config else ''
if 'until' not in config:
    OUTPUT = expand('{permanent}/{lang1}-{lang2}.{output_file}.gz', permanent=PERMANENT, lang1=LANG1, lang2=LANG2, output_file=OUTPUT_FILES)
    # this has to be added, because tokenisation rules are the last rules that are don't need both shards
    # otherwise snakemake waites for both shards be completed to continue
    OUTPUT.append(f'{DATADIR}/shards/03.split.{TRG_LANG}')
    OUTPUT.append(f'{DATADIR}/shards/03.split.{SRC_LANG}')

    # without these for some reason shards rules are done sequentially (snakemake doesn't recognize that is needs both shards output for rule all)
    OUTPUT.append(f'{DATADIR}/shards/02.batches.{LANG1}')
    OUTPUT.append(f'{DATADIR}/shards/02.batches.{LANG2}')
elif UNTIL == 'crawl':
    for domain, hosts in DOMAIN_2_HOSTS:
        for host in hosts:
            OUTPUT.append('{DATADIR}/warc/{host}/{CRAWLTARGET}.warc.gz')
elif UNTIL == 'preprocess':
    OUTPUT = expand('{datadir}/preprocess/{target}/{pproc}/{lang}/{pproc_file}', datadir=DATADIR, target=TARGETS, pproc=PPROC, langs=LANGS, pproc_file=PPROC_FILES)
elif UNTIL == 'shard':
    OUTPUT = expand('{datadir}/shards/02.batches.{lang}', datadir=DATADIR, lang=LANGS)
elif UNTIL == 'split':
    OUTPUT = expand('{datadir}/shards/03.split.{lang}', datadir=DATADIR, lang=LANGS)
elif UNTIL == 'translate':
    OUTPUT = f'{DATADIR}/shards/04.translate.{SRC_LANG}2{TRG_LANG}'
elif UNTIL == 'tokenise_trg':
    # TODO: decide where to put '0X_step' files (output of aggregate rules)
    OUTPUT = f'{DATADIR}/shards/05.tokenise.{TRG_LANG}'
elif UNTIL == 'tokenise_src':
    OUTPUT = f'{DATADIR}/shards/05.tokenise.{SRC_LANG}2{TRG_LANG}'
elif UNTIL == 'docalign':
    OUTPUT = f'{TRANSIENT}/06_01.docalign.{SRC_LANG}_{TRG_LANG}'
elif UNTIL == 'segalign':
    OUTPUT = f'{TRANSIENT}/06_02.segalign.{LANG1}_{LANG2}'
elif UNTIL == 'bifixer':
    OUTPUT = f'{TRANSIENT}/07_01.bifixer.{LANG1}_{LANG2}'
elif UNTIL == 'bicleaner':
    OUTPUT = f'{TRANSIENT}/07_02.bicleaner.{LANG1}_{LANG2}'
elif UNTIL == 'filter':
    OUTPUT = f'{TRANSIENT}/07_03.filter.{LANG1}_{LANG2}'
shell.prefix("set -euo pipefail;")
rule all:
    input: OUTPUT

#################################################################
# DAG will be re-evaluated after completing shard rule (because number of batches is dynamic and unknown)
checkpoint shard:
    # use url.gz as input to avoid having directories as input
    # input: expand("{datadir}/preprocess/{target}/{pproc}/{{lang}}/url.gz", datadir=DATADIR, target=TARGETS, pproc=PPROC)
    output: f'{DATADIR}/shards/02.batches.{{lang}}' # list of batches created for lang
    params:
        o = f'{DATADIR}/shards/{{lang}}'
    shell: '''
        ulimit -n 2048
        mkdir -p {params.o}
        for i in $(seq 1 4); do
            for j in $(seq 1 2); do
                mkdir -p {params.o}/$i/$j
                echo "plain_text $i $j" | base64 > {params.o}/$i/$j/plain_text
                echo "url $i $j" > {params.o}/$i/$j/url
                echo "mime $i $j" > {params.o}/$i/$j/mime
                gzip {params.o}/$i/$j/{{plain_text,mime,url}}
            done
        done
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
    input: f'{DATADIR}/shards/{{lang}}/{{shard}}/{{batch}}/plain_text.gz'
    output: f'{DATADIR}/shards/{{lang}}/{{shard}}/{{batch}}/sentences.gz'
    shell: '''
        zcat {input} | sed 's/ /:/g' | pigz -c > {output}
        '''

rule aggregate_split:
    input: lambda wildcards: [f'{batch}/sentences.gz' for batch in get_batches(wildcards.lang)]
    output: f'{DATADIR}/shards/03.split.{{lang}}'
    shell: ''' echo "{input}" | tr ' ' '\n' > {output} '''

#################################################################
### DOCALIGN ####################################################
def get_align_inputs(src_lang, trg_lang):
    src_batches = get_batches(src_lang)
    trg_batches = get_batches(trg_lang)

    iterator = product( [batch.split('/')[-2:] for batch in src_batches], [batch.split('/')[-2:] for batch in trg_batches] )
    # each input -> (shard, (src_batch, trg_batch))
    inputs = [(src_shard, (src_batch, trg_batch)) for ((src_shard, src_batch), (trg_shard, trg_batch)) in iterator if src_shard == trg_shard]
    return inputs

rule aggregate_matches:
    input: lambda wildcards: [f'{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{shard}/{SRC_LANG}{src_batch}_{TRG_LANG}{trg_batch}.06_01.matches' for (shard, (src_batch, trg_batch)) in get_align_inputs(SRC_LANG, TRG_LANG)]
    output: f'{TRANSIENT}/06_01.docalign.{SRC_LANG}_{TRG_LANG}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''
# MT ############################################################

rule mt_matches:
    input:
        l1=f'{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/sentences.gz',
        l2=f'{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/sentences.gz',
        u1=f'{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz',
        u2=f'{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz'
        # l1=rules.tokenise_translated.output,
        # l2=rules.tokenise_target.output
    output: f'{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.06_01.matches'
    params: folder=f'{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}'
    shell: ''' mkdir -p {params.folder}; join -t $'\t' -j3 <(paste <(zcat {input.l1}) <(zcat {input.u1})) <(paste <(zcat {input.l2}) <(zcat {input.u2})) -o 1.2,2.2,1.1,2.1 > {output} '''
# DIC ###########################################################
# TODO
#################################################################
### SEGALIGN ####################################################
rule aggregate_segalign:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{SRC_LANG}{src_batch}_{TRG_LANG}{trg_batch}.06_02.segalign.gz' for (shard, (src_batch, trg_batch)) in get_align_inputs(SRC_LANG, TRG_LANG)]
    output: f'{TRANSIENT}/06_02.segalign.{LANG1}_{LANG2}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''
# BLEUALIGN #####################################################
rule bleualign:
    input:
        indices=rules.mt_matches.output,
        # plain1=f'{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/sentences.gz',
        # plain2=f'{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/sentences.gz',
        # url1=f'{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz',
        # url2=f'{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz',
    params: folder=f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}'
    # in segalign rule output columns are reordered (or not) in accordance with translationDirection
    output:
        f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.06_02.segalign.gz'
    shell: '''
        mkdir -p {params.folder}
        paste <(cut -f 1,2 {input.indices}) <(cut -f 3 {input.indices} | base64 -d) <(cut -f 4 {input.indices} | base64 -d) | gzip -c > {output}
        '''
# HUNALIGN ######################################################
# TODO
#################################################################
### FILTERING AND CLEANING ######################################
filter_input = rules.bleualign.output

rule filter:
    input: filter_input
    output: temp(f'{TRANSIENT}/{LANG1}_{LANG2}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.07_03.filtered')
    shell: '''
        zcat {input} | LC_ALL=C sort -t $'\t' > {output}
        '''

rule aggregate_filter:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{SRC_LANG}{src_batch}_{TRG_LANG}{trg_batch}.07_03.filtered' for (shard, (src_batch, trg_batch)) in get_align_inputs(SRC_LANG, TRG_LANG)]
    output: f'{TRANSIENT}/07_03.filter.{LANG1}_{LANG2}'
    shell: ''' echo {input} | tr ' ' '\n' > {output} '''

raw_input_filename = '.'.join(filter_input[0].split('/')[-1].split('.')[1:]) # 06_02.segalign.gz / 07_01.bifixer / 07_02.bicleaner.gz

rule sents:
    input: lambda wildcards: [f'{TRANSIENT}/{LANG1}_{LANG2}/{shard}/{SRC_LANG}{src_batch}_{TRG_LANG}{trg_batch}.07_03.filtered' for (shard, (src_batch, trg_batch)) in get_align_inputs(SRC_LANG, TRG_LANG)]
    output: f'{PERMANENT}/{LANG1}-{LANG2}.sent.gz'
    shell: '''
        echo "Input is: {input}"
        LC_ALL=C sort -t $'\t' --compress-program=gzip -T {TMPDIR} --merge {input} | pigz -c > {output}
        '''

rule tmx:
    input: rules.sents.output
    output: f'{PERMANENT}/{LANG1}-{LANG2}.not-deduped.tmx.gz'
    shell: '''
        # if input are incorrect grep should fail
        zcat {input} | grep 'plain_text' | sed 's/ /_/g' | gzip -c > {output}
        '''
