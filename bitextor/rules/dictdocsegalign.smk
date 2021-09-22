#################################################################
### DOCALIGN ####################################################
# DICTIONARY-BASED ##############################################
rule build_idx:
    """
    Produce an index of words used in text1 and text2
    :input.text1: gz-compressed file with a base64-encoded tokenised documents in SRC_LANG per line
    :input.text2: gz-compressed file with a base64-encoded tokenised documents in TRG_LANG per line
    :output: gz-commpressed index file, format is <lang> \\t <word> \\t <doc_id1>[:<increment> ...]
    """
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
    """
    Read .idx file and produce an ridx file corresponding to preliminary alignment by computing bag-of-words overlap metric
        i.e. SRC_LANG docs and their corresponding n-best TRG_LANG canidates to be parallel

        from this point until final alignment step, document indices for SRC_LANG will range from 1 to number of SRC docs,
        document indices for TRG_LANG will range from (number of SRC docs + 1) to (number of SRC docs + number of TRG docs)
    :input.idx: gz-compressed index file, output of build_idx rule
    :input.dic: SRC_LANG-TRG_LANG dictionary provided by user
    :output: gz-compressed ridx file, format is <doc_id> \\t <doc_id>:<score> [ \\t <doc_id>:<score> ...]
    """
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
    """
    Read .idx file and produce an ridx file corresponding to preliminary alignment by computing bag-of-words overlap metric
        i.e. TRG_LANG docs and their corresponding n-best SRC_LANG canidates to be parallel

        from this point until final alignment step, document indices for SRC_LANG will range from 1 to number of SRC docs,
        document indices for TRG_LANG will range from (number of SRC docs + 1) to (number of SRC docs + number of TRG docs)
    :input.idx: gz-compressed index file, output of build_idx rule
    :input.dic: TRG_LANG-SRC_LANG dictionary provided by user
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1> [ \\t <doc_id_n>:<f1> ...],
        where f1 is the [0.0, 1.0] score corresponding to the metric computed in this step
    """
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
    """
    For each candidate pair in the input ridx file, compute metric corresponding to images overlapping
    :input.ridx: gz-compressed {1,2}.ridx, output of idx2ridx step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1>:<f2> [ \\t <doc_id_n>:<f1>:<f2> ...],
        where f1 is the score computed in previous step, and f2 is newly computed images overlap score
        all the scores are in [0.0, 1.0] range
    """
    input:
        ridx=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.ridx.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.imgoverlap.gz"),
    shell:
        """
        zcat {input.ridx} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_imagesetoverlap.py --html1 {input.html1} --html2 {input.html2} \
            | gzip -c > {output}
        """


rule imagesetoverlap2structuredistance:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to HTML structure similarity
    :input.ridx: gz-compressed {1,2}.ridx, output of ridx2imagesetoverlap step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1>:<f2>:<f3> [ \\t <doc_id_n>:<f1>:<f2>:<f3> ...],
        where f1-f2 are the scores computed in previous steps, and f3 is newly computed structuer distance score
        all the scores are in [0.0, 1.0] range
    """
    input:
        imagesetoverlap=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.imgoverlap.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.structuredistance.gz"),
    shell:
        """
        zcat {input.imagesetoverlap} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_structuredistance.py --html1 {input.html1} --html2 {input.html2} \
            | gzip -c > {output}
        """


rule structuredistance2urldistance:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to the similarity of URLs used in the documents
    :input.ridx: gz-compressed {1,2}.ridx, output of imagesetoverlap2structuredistance step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1>...<f3>:<f4> [ \\t <doc_id_n>:<f1>...<f3>:<f4> ...],
        where f1-f3 are the scores computed in previous steps, and f4 is newly computed url distance score
        all the scores are in [0.0, 1.0] range
    """
    input:
        structuredistance=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.structuredistance.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urldistance.gz"),
    priority: 8
    shell:
        """
        zcat {input.structuredistance} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urlsdistance.py \
                --html1 {input.html1} --html2 {input.html2} \
                --url1 {input.url1} --url2 {input.url2} \
            | gzip -c > {output}
        """


rule urldistance2mutuallylinked:
    """
    For each candidate pair in the input ridx file, compute whether a pair of documents link to each other or not
    :input.ridx: gz-compressed {1,2}.ridx, output of structuredistance2urldistance step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1>...<f4>:<f5> [ \\t <doc_id_n>:<f1>...<f4>:<f5> ...],
        where f1-f4 are the scores computed in previous steps, and f5 is newly computed mutually linked score
        all the scores are in [0.0, 1.0] range
    """
    input:
        urldistance=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urldistance.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.mutuallylinked.gz"),
    shell:
        """
        zcat {input.urldistance} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_mutuallylinked.py \
                --html1 {input.html1} --html2 {input.html2} \
                --url1 {input.url1} --url2 {input.url2} \
            | gzip -c > {output}
        """


rule mutuallylinked2urlscomparison:
    """
    For each candidate pair in the input ridx file, compute editing distance of their urls
    :input.ridx: gz-compressed {1,2}.ridx, output of urldistance2mutuallylinked step
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1>...<f5>:<f6> [ \\t <doc_id_n>:<f1>...<f5>:<f6> ...],
        where f1-f5 are the scores computed in previous steps, and f6 is newly urls comparison score
        all the scores are in [0.0, 1.0] range
    """
    input:
        mutuallylinked=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.mutuallylinked.gz",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        temp(f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlscomparison.gz"),
    shell:
        """
        zcat {input.mutuallylinked} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urlscomparison.py --url1 {input.url1} --url2 {input.url2} \
            | gzip -c > {output}
        """


rule urlscomparison2urlsoverlap:
    """
    For each candidate pair in the input ridx file, compute metric corresponding to urls overlapping
    :input.ridx: gz-compressed {1,2}.ridx, output of mutuallylinked2urlscomparison step
    :input.html1: gz-compressed file with a base64-encoded html documents in SRC_LANG per line
    :input.html2: gz-compressed file with a base64-encoded html documents in TRG_LANG per line
    :output: gz-compressed ridx file, format is <doc_id_1> \\t <doc_id_2>:<f1>...<f6>:<f7> [ \\t <doc_id_n>:<f1>...<f6>:<f7> ...],
        where f1-f6 are the scores computed in previous steps, and f7 is newly urls overlapping score
        all the scores are in [0.0, 1.0] range
    """
    input:
        urlscomparison=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlscomparison.gz",
        html1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/{HTML_FILE}",
        html2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/{HTML_FILE}",
    output:
        # not marking this as temp because this is the file that contains all the features
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{{num}}.urlsoverlap.gz",
    shell:
        """
        zcat {input.urlscomparison} \
            | {PROFILING} python3 {WORKFLOW}/docalign/features/bitextor_urlsetoverlap.py --html1 {input.html1} --html2 {input.html2} \
            | gzip -c > {output}
        """


rule urlsoverlap2rank:
    """
    For each candidate pair in the input ridx file predict the probability of the documents being parallel
    :input:  gz-compressed {1,2}.ridx, output of urlscomparison2urlsoverlap step that contains all the features
    :output: gz-compressed file with ranked pairs, format is <doc_id_1> \\t <doc_id_2>:<score> [ \\t <doc_id_n>:<score> ...],
        candidate documents ordered by score
    """
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
    """
    Process bidirectional alignment rankings and produce a list of best matches
    :input.rank1: gz-compressed ranked pairs files, output of urlsoverlap2rank for SRC_LANG-TRG_LANG direction
    :input.rank2: gz-compressed ranked pairs files, output of urlsoverlap2rank for TRG_LANG-SRC_LANG direction
    :input.url1: gz-compressed file with a SRC document URL per line
    :input.url2: gz-compressed file with a TRG document URL per line
    :output: indices file
        plain text file with 2 tab separated columns: src_index, trg_index
            src_index is the number of the aligned document in source language
            trg_index is the number of the aligned document in target language
    """
    input:
        rank1=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.1.rank.gz",
        rank2=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.2.rank.gz",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.bitextor.06_01.matches",
    shell:
        """
        {PROFILING} python3 {WORKFLOW}/docalign/bitextor_align_documents.py \
            --lines1 $(zcat {input.url_l1} | wc -l) --lines2 $(zcat {input.url2} | wc -l) \
            -n 1 -i converge {input.rank1} {input.rank2} > {output}
        """


#################################################################
### SEGALIGN ####################################################
# HUNALIGN ######################################################

docalign_str = ""  # Default: externalMT as docalign

if DOCALIGN == "DIC":
    docalign_str = "bitextor."


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


rule matches2hunalign:
    """
    Use hunalign to align sentences withing the matched documents
    :input.indices: output of docalign (columns are "score src_index trg_index" or "src_index trg_index")
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
        indices=f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.{docalign_str}06_01.matches",
        plain1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/sentences.gz",
        plain2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/sentences.gz",
        url1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/url.gz",
        url2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/url.gz",
        tok1=f"{DATADIR}/shards/{SRC_LANG}/{{shard}}/{{src_batch}}/tokenised.gz",
        tok2=f"{DATADIR}/shards/{TRG_LANG}/{{shard}}/{{trg_batch}}/tokenised.gz",
        hunaligndic=rules.hunaligndic.output,
    output:
        f"{TRANSIENT}/{SRC_LANG}_{TRG_LANG}/{{shard}}/{SRC_LANG}{{src_batch}}_{TRG_LANG}{{trg_batch}}.hunalign.06_02.segalign.gz",
    params:
        c1=1 if DOCALIGN == "DIC" else 2,
        c2=2 if DOCALIGN == "DIC" else 3,
    shell:
        """
        cut -f {params.c1},{params.c2} {input.indices} \
            | LC_ALL=C sort -nk1,1 -t $'\t' \
            | python3 {WORKFLOW}/docalign/bitextor_build_docalign.py \
                --columns1 {input.url1} {input.plain1} {input.tok1} --columns2 {input.url2} {input.plain2} {input.tok2} \
            | awk -F\'\t\' \'{{print $2,$6,$3,$7,$4,$8}}\' OFS=\'\t\' \
            | {PROFILING} python3 {WORKFLOW}/bitextor_align_segments.py {DEFERRED} {MMHSUM_PATH} -d {input.hunaligndic} -t {TMPDIR} \
                --hunalign "hunalign" --hunalign-thresh {SEGALIGN_THRESHOLD} \
            | gzip -c > {output}
        """
