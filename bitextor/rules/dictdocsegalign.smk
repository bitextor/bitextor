#################################################################
### DOCALIGN ####################################################
# DICTIONARY-BASED ##############################################
rule build_idx:
    input:
        text1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/tokenised.gz",
        text2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/tokenised.gz",
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.idx.gz",
    shell:
        """
        {PROFILING} python3 {WORKFLOW}/docalign/bitextor_buildidx.py --lang1 {SRC_LANG} --lang2 {TRG_LANG} -m 15 --text1 {input.text1} --text2 {input.text2} \
                | gzip -c > {output}
        """


rule idx2ridx_src2trg:
    input:
        idx=rules.build_idx.output,
        dic=expand("{dic}", dic=DIC),
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.1.ridx.gz",
    shell:
        """
        zcat {input.idx} \
            | {PROFILING} python3 {WORKFLOW}/docalign/bitextor_idx2ridx.py -d {input.dic} --lang1 {SRC_LANG} --lang2 {TRG_LANG} \
            | gzip -c > {output}
        """


rule idx2ridx_trg2src:
    input:
        idx=rules.build_idx.output,
        dic=expand("{dic}", dic=DIC),
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.2.ridx.gz",
    shell:
        """
        zcat {input.idx} \
            | {PROFILING} python3 {WORKFLOW}/docalign/bitextor_idx2ridx.py -d {input.dic} --lang1 {TRG_LANG} --lang2 {SRC_LANG} \
            | gzip -c > {output}
        """


rule ridx2imagesetoverlap:
    input:
        ridx=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.ridx.gz",
        debpl_html_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        debpl_html_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.imgoverlap.gz"),
    shell:
        """
        zcat {input.ridx} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_imagesetoverlap.py --html1 {input.debpl_html_l1} --html2 {input.debpl_html_l2} \
            | gzip -c > {output}
        """


rule imagesetoverlap2structuredistance:
    input:
        imagesetoverlap=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.imgoverlap.gz",
        debpl_html_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        debpl_html_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.structuredistance.gz"),
    shell:
        """
        zcat {input.imagesetoverlap} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_structuredistance.py --html1 {input.debpl_html_l1} --html2 {input.debpl_html_l2} \
            | gzip -c > {output}
        """


rule structuredistance2urldistance:
    input:
        structuredistance=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.structuredistance.gz",
        debpl_html_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        debpl_html_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
        url_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urldistance.gz"),
    priority: 8
    shell:
        """
        zcat {input.structuredistance} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urlsdistance.py \
                --html1 {input.debpl_html_l1} --html2 {input.debpl_html_l2} \
                --url1 {input.url_l1} --url2 {input.url_l2} \
            | gzip -c > {output}
        """


rule urldistance2mutuallylinked:
    input:
        urldistance=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urldistance.gz",
        debpl_html_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        debpl_html_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
        url_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.mutuallylinked.gz"),
    shell:
        """
        zcat {input.urldistance} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_mutuallylinked.py \
                --html1 {input.debpl_html_l1} --html2 {input.debpl_html_l2} \
                --url1 {input.url_l1} --url2 {input.url_l2} \
            | gzip -c > {output}
        """


rule mutuallylinked2urlscomparison:
    input:
        mutuallylinked=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.mutuallylinked.gz",
        url_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlscomparison.gz"),
    shell:
        """
        zcat {input.mutuallylinked} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urlscomparison.py --url1 {input.url_l1} --url2 {input.url_l2} \
            | gzip -c > {output}
        """


rule urlscomparison2urlsoverlap:
    input:
        urlscomparison=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlscomparison.gz",
        debpl_html_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        debpl_html_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        # not marking this as temp because this is the file that contains all the features
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlsoverlap.gz",
    shell:
        """
        zcat {input.urlscomparison} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urlsetoverlap.py --html1 {input.debpl_html_l1} --html2 {input.debpl_html_l2} \
            | gzip -c > {output}
        """


rule urlsoverlap2rank:
    input:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlsoverlap.gz",
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.rank.gz",
    shell:
        """
        zcat {input} \
            | {PROFILING} python3 {WORKFLOW}/docalign/bitextor_rank.py -m {WORKFLOW}/data/model/docalign.svm.classifier \
            | gzip -c > {output}
        """


rule aligndocumentsBitextor:
    input:
        rank1=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.1.rank.gz",
        rank2=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.2.rank.gz",
        url_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.bitextor.06_01.matches",
    shell:
        """
        {PROFILING} python3 {WORKFLOW}/docalign/bitextor_align_documents.py \
            --lines1 $(zcat {input.url_l1} | wc -l) --lines2 $(zcat {input.url_l2} | wc -l) \
            -n 1 -i converge {input.rank1} {input.rank2} > {output}
        """


#################################################################
### SEGALIGN ####################################################
# HUNALIGN ######################################################

docalign_str = ""  # Default: externalMT as docalign

if DOCALIGN == "DIC":
    docalign_str = "bitextor."


rule matches2hunalign:
    input:
        indices=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{docalign_str}06_01.matches",
        plain1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/sentences.gz",
        plain2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/sentences.gz",
        url_l1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url_l2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
        tok1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/tokenised.gz",
        tok2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/tokenised.gz",
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.hunalign.docalign.{docalign_str}06_02.matches.gz",
    params:
        c1=1 if DOCALIGN == "DIC" else 2,
        c2=2 if DOCALIGN == "DIC" else 3,
    shell:
        """
        cut -f {params.c1},{params.c2} {input.indices} \
            | LC_ALL=C sort -nk1 \
            | {PROFILING} python3 {WORKFLOW}/docalign/bitextor_build_docalign.py \
                --columns1 {input.url_l1} {input.plain1} {input.tok1} --columns2 {input.url_l2} {input.plain2} {input.tok2} \
            | awk -F\'\t\' \'{{print $2,$6,$3,$7,$4,$8}}\' OFS=\'\t\' \
            | gzip -c -f > {output} # Format: url1 <tab> url2 <tab> text1 <tab> text2 <tab> tok1 <tab> tok2
        """


rule hunaligndic:
    input:
        expand("{dic}", dic=DIC),
    output:
        f"{DATADIR}/hunalign_dic",
    run:
        with open(output[0], "wt") as outw:
            with open(input[0], "rt") as inr:
                header = inr.readline().strip()
                langs = header.split("\t")
                if langs[0] == LANG1 and langs[1] == LANG2:
                    inverse = True
                else:
                    inverse = False
                for inline in inr:
                    columns = inline.strip().split("\t")
                    if inverse:
                        outw.write(f"{columns[1]} @ {columns[0]}\n")
                    else:
                        outw.write(f"{columns[0]} @ {columns[1]}\n")


rule alignsegments_hunalign:
    input:
        hunaligndic=rules.hunaligndic.output,
        hunalign_matches=rules.matches2hunalign.output,
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.hunalign.06_02.segalign.gz",
    shell:
        """
        zcat {input.hunalign_matches} \
            | {PROFILING} python3 {WORKFLOW}/bitextor_align_segments.py {DEFERRED} {MMHSUM_PATH} -d {input.hunaligndic} -t {TMPDIR} \
                --hunalign "hunalign" --hunalign-thresh {SEGALIGN_THRESHOLD} \
            | gzip -c > {output}
        """
