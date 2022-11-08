
# Notice that: SRC_LANG = LANG1 and TRG_LANG = LANG2

#################################################################
### DOCALIGN ####################################################
# DICTIONARY-BASED ##############################################
rule build_idx:
    """
    Produce an index of words used in text1 and text2
    :input.text1: gz-compressed file with a base64-encoded tokenised documents in SRC_LANG per line
    :input.text2: gz-compressed file with a base64-encoded tokenised documents in TRG_LANG per line
    :output: gz-commpressed index file, format is <lang> \\t <word> \\t <doc_id_[src|trg]>
    """
    input:
        text1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/tokenised.gz",
        text2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/tokenised.gz",
    output:
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.idx.gz",
    shell:
        """
        {PROFILING} python3 {WORKFLOW}/docalign/bitextor_build_idx.py --lang1 {SRC_LANG} --lang2 {TRG_LANG} \
            -m 15 --text1 {input.text1} --text2 {input.text2} \
            | gzip -c > {output}
        """


rule idx2ridx:
    """
    Read .idx file and produce an ridx file corresponding to preliminary alignment by computing bag-of-words overlap metric
        i.e. [SRC|TRG]_LANG docs and their corresponding n-best [TRG|SRC]_LANG candidates to be parallel
    :input.idx: gz-compressed index file, output of build_idx rule
    :input.dic: SRC_LANG-TRG_LANG dictionary provided by user
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <score>
    """
    input:
        idx=rules.build_idx.output,
        dic=expand("{dic}", dic=DIC),
    output:
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.ridx.gz",
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--lang1 {SRC_LANG} --lang2 {TRG_LANG}" || \
                    echo "--lang1 {TRG_LANG} --lang2 {SRC_LANG}")

        zcat {input.idx} \
            | {PROFILING} python3 {WORKFLOW}/docalign/bitextor_idx2ridx.py -d {input.dic} $params \
            | gzip -c > {output}
        """


rule image_set_overlap:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to images overlapping
    :input.ridx: gz-compressed {src2trg,trg2src}.ridx, output of idx2ridx step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <f1> \\t <f2>,
        where f1 is the score computed in previous step, and f2 is newly computed images overlap score
        all the scores are in [0.0, 1.0] range
    """
    input:
        ridx=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.ridx.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        temp(f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.imgoverlap.gz"),
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--html1 {input.html1} --html2 {input.html2}" || \
                    echo "--html1 {input.html2} --html2 {input.html1}")

        zcat {input.ridx} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_image_set_overlap.py $params \
            | gzip -c > {output}
        """


rule structure_distance:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to HTML structure similarity
    :input.ridx: gz-compressed {src2trg,trg2src}.ridx, output of image_set_overlap step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <f1> \\t <f2> \\t <f3>,
        where f1-f2 are the scores computed in previous steps, and f3 is newly computed structuer distance score
        all the scores are in [0.0, 1.0] range
    """
    input:
        imagesetoverlap=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.imgoverlap.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        temp(f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.structuredistance.gz"),
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--html1 {input.html1} --html2 {input.html2}" || \
                    echo "--html1 {input.html2} --html2 {input.html1}")

        zcat {input.imagesetoverlap} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_structure_distance.py $params \
            | gzip -c > {output}
        """


rule url_distance:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to the similarity of URLs used in the documents
    :input.ridx: gz-compressed {src2trg,trg2src}.ridx, output of structure_distance step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <f1> \\t <f2> \\t ... \\t <f4>,
        where f1-f3 are the scores computed in previous steps, and f4 is newly computed url distance score
        all the scores are in [0.0, 1.0] range
    """
    input:
        structuredistance=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.structuredistance.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.urldistance.gz"),
    priority: 8
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--html1 {input.html1} --html2 {input.html2} --url1 {input.url1} --url2 {input.url2}" || \
                    echo "--html1 {input.html2} --html2 {input.html1} --url1 {input.url2} --url2 {input.url1}")

        zcat {input.structuredistance} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urls_distance.py $params \
            | gzip -c > {output}
        """


rule mutually_linked:
    """
    For each candidate pair in the input ridx file, compute whether a pair of documents link to each other or not
    :input.ridx: gz-compressed {src2trg,trg2src}.ridx, output of url_distance step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <f1> \\t <f2> \\t ... \\t <f5>,
        where f1-f4 are the scores computed in previous steps, and f5 is newly computed mutually linked score
        all the scores are in [0.0, 1.0] range
    """
    input:
        urldistance=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.urldistance.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.mutuallylinked.gz"),
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--html1 {input.html1} --html2 {input.html2} --url1 {input.url1} --url2 {input.url2}" || \
                    echo "--html1 {input.html2} --html2 {input.html1} --url1 {input.url2} --url2 {input.url1}")

        zcat {input.urldistance} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_mutually_linked.py $params \
            | gzip -c > {output}
        """


rule urls_comparison:
    """
    For each candidate pair in the input ridx file, compute editing distance of their urls
    :input.ridx: gz-compressed {src2trg,trg2src}.ridx, output of mutually_linked step
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <f1> \\t <f2> \\t ... \\t <f6>,
        where f1-f5 are the scores computed in previous steps, and f6 is newly urls comparison score
        all the scores are in [0.0, 1.0] range
    """
    input:
        mutuallylinked=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.mutuallylinked.gz",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.urlscomparison.gz"),
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--url1 {input.url1} --url2 {input.url2}" || \
                    echo "--url1 {input.url2} --url2 {input.url1}")

        zcat {input.mutuallylinked} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urls_comparison.py $params \
            | gzip -c > {output}
        """


rule urls_overlap:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to urls overlapping
    :input.ridx: gz-compressed {src2trg,trg2src}.ridx, output of urls_comparison step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :output: gz-compressed ridx file, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <f1> \\t <f2> \\t ... \\t <f7>,
        where f1-f6 are the scores computed in previous steps, and f7 is newly urls overlapping score
        all the scores are in [0.0, 1.0] range
    """
    input:
        urlscomparison=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.urlscomparison.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        # not marking this as temp because this is the file that contains all the features
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.urlsoverlap.gz",
    shell:
        """
        params=$([[ {wildcards.direction} == src2trg* ]] && \
                    echo "--html1 {input.html1} --html2 {input.html2}" || \
                    echo "--html1 {input.html2} --html2 {input.html1}")

        zcat {input.urlscomparison} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_url_set_overlap.py $params \
            | gzip -c > {output}
        """


rule ranking:
    """
    For each candidate pair in the input ridx file predict the probability of the documents being parallel
    :input:  gz-compressed {src2trg,trg2src}.ridx, output of urls_overlap step that contains all the features
    :output: gz-compressed file with ranked pairs, format is <doc_id_[src|trg]> \\t <doc_id_[trg|src]> \\t <score>,
        candidate documents ordered by score
    """
    input:
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.urlsoverlap.gz",
    output:
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{direction}}.rank.gz",
    shell:
        """
        zcat {input} \
            | {PROFILING} python3 {WORKFLOW}/docalign/bitextor_rank.py -m {WORKFLOW}/data/model/docalign.svm.classifier \
            | gzip -c > {output}
        """


rule align_documents:
    """
    Process bidirectional alignment rankings and produce a list of best matches
    :input.rank1: gz-compressed ranked pairs files, output of ranking for SRC_LANG-TRG_LANG direction
    :input.rank2: gz-compressed ranked pairs files, output of ranking for TRG_LANG-SRC_LANG direction
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: indices file
        plain text file with 2 tab separated columns: src_index, trg_index
            src_index is the number of the aligned document in source language
            trg_index is the number of the aligned document in target language
    """
    input:
        rank1=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.src2trg_{SRC_LANG}2{TRG_LANG}.rank.gz",
        rank2=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.trg2src_{TRG_LANG}2{SRC_LANG}.rank.gz",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.DIC.06_01.matches",
    shell:
        """
        {PROFILING} python3 {WORKFLOW}/docalign/bitextor_align_documents.py \
            --lines1 $(zcat {input.url1} | wc -l) --lines2 $(zcat {input.url2} | wc -l) \
            --threshold {DOC_THRESHOLD} -n 1 -i converge --print-score \
            {input.rank1} {input.rank2} > {output}
        """


#################################################################
### SEGALIGN ####################################################
# HUNALIGN ######################################################


rule create_hunalign_dic_format:
    input:
        expand("{dic}", dic=DIC),
    output:
        f"{DATADIR}/hunalign_dic",
    run:
        with open(output[0], "wt") as outw:
            with open(input[0], "rt") as inr:
                header = inr.readline().strip()
                langs = header.split("\t")
                if langs[0] == SRC_LANG and langs[1] == TRG_LANG:
                    inverse = True
                else:
                    inverse = False
                for inline in inr:
                    columns = inline.strip().split("\t")
                    if inverse:
                        outw.write(f"{columns[1]} @ {columns[0]}\n")
                    else:
                        outw.write(f"{columns[0]} @ {columns[1]}\n")


rule hunalign:
    """
    Use hunalign to align sentences withing the matched documents
    :input.indices: output of docalign (columns are "mt_doc_aligner_score idx_translated idx_trg" or "src_index trg_index")
    :input.plain1: gz-compressed file with a base64-encoded document per line
        sentence-split source documents
    :input.plain1: gz-compressed file with a base64-encoded document per line
        sentence-split target documents
    :input.url1: gz-compressed file with a URL per line for source (source of the corresponding documents)
    :input.url2: gz-compressed file with a URL per line for target (source of the corresponding documents)
    :input.tok1: gz-compressed file with a base64-encoded tokenised SRC document per line
    :input.tok2: gz-compressed file with a base64-encoded tokenised TRG document per line
    :output: aligned sentences
        gz-compressed file with 5 tab-separated columns: url1,url2,sentence1,sentence2,score
    """
    input:
        indices=f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{DOCALIGN}.06_01.matches",
        plain1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/sentences.gz",
        plain2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/sentences.gz",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
        tok1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/tokenised.gz",
        tok2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/tokenised.gz",
        hunaligndic=rules.create_hunalign_dic_format.output,
    output:
        f"{TRANSIENT}/{LANG1}_{LANG2}/shards/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.hunalign.06_02.segalign.gz",
    params:
        c1="src_index" if DOCALIGN == "DIC" else "src_idx" if DOCALIGN == "NDA" else "idx_translated",
        c2="trg_index" if DOCALIGN == "DIC" else "trg_idx" if DOCALIGN == "NDA" else "idx_trg",
        deferred=f"--print-sent-hash \"{DEFERRED_CMD}\"" if DEFERRED else '',
        metadata=apply_format(','.join(PROPAGATE_METADATA_HEADERS), "--metadata-header-fields {}"),
        src_metadata=f"-l {DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/metadata.gz" if PROPAGATE_METADATA_FROM_TEXT else '',
        trg_metadata=f"-r {DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/metadata.gz" if PROPAGATE_METADATA_FROM_TEXT else '',
    shell:
        """
        header="src_url\ttrg_url\tsrc_text\ttrg_text\tsrc_tokenized\ttrg_tokenized"

        [[ ! -z "{params.metadata}" ]] && header=$(echo "${header}\tsrc_metadata\ttrg_metadata")

        python3 {WORKFLOW}/utils/cut_header.py -f {params.c1},{params.c2} --input {input.indices} \
            | tail -n +2 \
            | docjoin \
                -l {input.url1} -r {input.url2} \
                -l {input.plain1} -r {input.plain2} \
                -l {input.tok1} -r {input.tok2} {params.src_metadata} {params.trg_metadata} \
            | cat <(echo "$header") - \
            | {PROFILING} python3 {WORKFLOW}/bitextor_align_segments.py {params.deferred} -d {input.hunaligndic} \
                -t {TMPDIR} --hunalign "hunalign" --hunalign-thresh {SEGALIGN_THRESHOLD} {params.metadata} - \
            | gzip -c > {output}
        """
