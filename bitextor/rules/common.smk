"""
Some helper functions used in the main Snakefile,
as per the recommendation of Snakemake devs
https://snakemake.readthedocs.io/en/stable/snakefiles/best_practices.html#best-practices
"""
import os
import tldextract
import validators
from itertools import product


"""
Group hosts by domain
:param hosts: a list of hosts
:returns: a dictionary with domains as keys and lists of related hosts as values
"""
def create_domain_key_2_host_map(hosts):
    key2hosts = {}
    badhosts = []
    for host in hosts:
        if not validators.domain(host):
            badhosts.append(host)
            continue

        # don't merge blog sites
        if host.find(".blogspot.") >= 0 or host.find(".wordpress.") >= 0:
            key = host
        else:
            key = tldextract.extract(host).domain

        if key not in key2hosts:
            key2hosts[key] = []
        key2hosts[key].append(host)
    if badhosts:
        raise ValueError("ERROR: Some hosts are not valid: \n%s" % ("\n".join(badhosts)))
    return key2hosts


"""
Check the validity of WARCs (i.e. that they exist on disk),
    and assign an ID to each valid WARC, so that each WARC is processed individually
:param warcs: a list of warcs provided by the user
:returns: a dictionary with IDs as keys as a WARCs as values
"""
def create_id_key_2_warc_map(warcs):
    bad_warcs=[]
    id2warc = {}
    for i, warc in enumerate(warcs):
        warc = os.path.expanduser(warc)
        if not os.path.isfile(warc):
            bad_warcs.append(warc)
        else:
            id2warc[f"{i}"] = warc
    if bad_warcs:
        bad_warcs_msg = "\n".join(bad_warcs)
        raise ValueError(f"ERROR: Some WARCs could not be found:\n{bad_warcs_msg}")
    return id2warc

"""
Obtains a list of WARCs produced by linguacrawl checkpoint
:returns: a list of paths to generated WARCs
"""
def get_linguacrawl_warcs():
    warcs = []
    with checkpoints.linguacrawl_download.get().output[0].open() as f:
        for line in f:
            warcs.append(line.strip())
    return warcs


"""
Retuns the list of WARCs for the current target
:param target2warcs: a dictionary that relates each target to the provided WARCS
    (either existing, or be generated by the crawler)
:param target: either the domain or the ID of a WARC provided by the user
:returns: a list of WARCs corresponding to the target
"""
def get_warcs_names(target):
    if CRAWLTARGET == "linguacrawl":
        warcs = get_linguacrawl_warcs()
        return warcs, list(map(lambda warc: warc.split("/")[-1], warcs))

    return TARGET_2_WARCS[target].split("/")[-1]


"""
Get the input of the preprocessing rules
:param target2warcs: a dictionary that relates each target to the provided WARCS
    (either existing, or be generated by the crawler)
:param target: either the domain or the ID of a WARC provided by the user
:returns: a list of WARCs that should be processed by a single preprocess rule
"""
def get_pproc_input(wildcards):
    target = wildcards.target
    if CRAWLTARGET == "linguacrawl" and len(HOSTS) != 0:
        try:
            # Retrieve warcs names, that in first instance will fail because linguacrawl has not been executed yet
            warcs, warcs_targets = get_warcs_names(target)
        except snakemake.exceptions.IncompleteCheckpointException:
            # We have received a provided warc (likely)
            try:
                # Check if the requested pproc input is a provided warc
                return TARGET_2_WARCS[target]
            except:
                # Should not happen, but force exception again in case something went wrong (it might continue if linguacrawl finished)
                warcs, warcs_targets = get_warcs_names(target)

        for warc in warcs:
            if not os.path.exists(warc):
                warcs_targets.remove(warc.split("/")[-1])
                sys.stderr.write(f"WARNING: non-existent WARC ({warc}) detected and fixed\n")

        try:
            # Check if the requested pproc input is a provided warc
            return TARGET_2_WARCS[target]
        except:
            pass

        return warcs[warcs_targets.index(target)]

    return TARGET_2_WARCS[target]


"""
Obtain shard input corresponding to the preprocessed crawled WARCs
"""
def get_shard_input_crawled(lang):
    if CRAWLTARGET == "linguacrawl" and len(HOSTS) != 0:
        warcs_path, warcs = get_warcs_names(None)

        for warc in warcs_path:
            if not os.path.exists(warc):
                warcs.remove(warc.split("/")[-1])
                sys.stderr.write(f"WARNING: non-existent WARC ({warc}) detected and fixed\n")

        return expand(
            "{datadir}/preprocess/{target}/{pproc}/{{lang}}/url.gz",
            datadir=DATADIR,
            target=warcs,
            pproc=PPROC
        )

    # crawler != linguacrawl
    return expand(
        "{datadir}/preprocess/{target}/{pproc}/{{lang}}/url.gz",
        datadir=DATADIR,
        target=list(TARGET_2_CRAWLED_WARCS.keys()),
        pproc=PPROC
    )


"""
Obtain shard input corresponding to the preprocessed WARCs provided by the user
"""
def get_shard_input_warcs(lang):
    return expand(
        "{datadir}/preprocess/{target}/{pproc}/{{lang}}/url.gz",
        datadir=DATADIR,
        target=list(TARGET_2_PROVIDED_WARCS.keys()),
        pproc=PPROC
    )


"""
Obtain a list of batches for a language
:param lang: target language
:returns: a list of batches, i.e. paths of each batch folder
"""
def get_batches(lang):
    batches = []
    with checkpoints.shard.get(lang=lang).output[0].open() as f:
        for line in f:
            batches.append(line.strip())
    return batches


"""
Obtain a list of potprocessing batches/chunks (paths to batch files without the extension)
:returns: a list of IDs of batches (just the numbers)
"""
def get_postproc_batches():
    batches = []
    with checkpoints.split_segalign.get().output.batches.open() as f:
        for line in f:
            batches.append(line.strip().split("/")[-1])  # obtain just the number of the chunks
    return batches


"""
Retuns a product of batches within shards
:param src_batches: a list of batches,
    i.e. paths of each batch folder for source language
:param trg_batches: a list of batches,
    i.e. paths of each batch folder for target language
:returns a list of tuples of (shard, (src_batch, trg_batch))
"""
def get_mt_docalign_inputs(src_batches, trg_batches):
    # product( [[shard, batch], [shard, batch], ...], [[shard, batch], [shard, batch], ...] )
    iterator = product(
        [batch.split("/")[-2:] for batch in src_batches],
        [batch.split("/")[-2:] for batch in trg_batches]
    )
    # each item -> (shard, (src_batch, trg_batch))
    return [
        (src_shard, (src_batch, trg_batch))
        for ((src_shard, src_batch), (trg_shard, trg_batch)) in iterator
        if src_shard == trg_shard
    ]


"""
Return the list of batches pairs that have to be aliged,
    i.e. that product of batches within shards
:param src_lang: source language of the alignment
:param trg_lang: target language of the alignment
:returns: a list of tuples of (shard, (src_batch, trg_batch))
"""
def get_align_inputs(src_lang, trg_lang):
    src_batches = get_batches(src_lang)
    trg_batches = get_batches(trg_lang)
    # each input -> (shard, (src_batch, trg_batch))
    inputs = get_mt_docalign_inputs(src_batches, trg_batches)
    return inputs


"""
Get approriate script for a language

:param scripts_dict: a dicionary a script per language/default
:param language: target language
:returns: extracted script, or empty string if not found
"""
def get_lang_or_default(scripts_dict, language):
    cmd = ""
    if language in scripts_dict:
        cmd = scripts_dict[language]
    elif "default" in scripts_dict:
        cmd = scripts_dict["default"]
    return cmd


"""
Get custom non-breaking prefix from provided dictionary

:param nbp_dict: dictionary with the nbp files per language
:param language: target language
:returns: extracted script, or empty string if not found
"""
def get_customnbp(nbp_dict, language):
    nbp = ""
    if language in nbp_dict:
        nbp = nbp_dict[language]
    return nbp


"""
Helper function to format parameters to pass to scripts
"""
def apply_format(string, replace_format, replace_token="{}", replace_only_if_true=True):
    if replace_only_if_true and string or not replace_only_if_true:
        return replace_format.replace(replace_token, string)

    return string
