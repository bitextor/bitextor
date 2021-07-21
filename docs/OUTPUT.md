# Bitextor output

Bitextor generates the final parallel corpora in multiple formats. These files will be placed in `permanentDir` folder and will have the following format: `{lang1}-{lang2}.{prefix}.gz`, where `{prefix}` corresponds to a descriptor of the corresponding format.

## Default

The files that will be always generated (regardless of configuration) are `{lang1}-{lang2}.raw.gz` and `{lang1}-{lang2}.sent.gz`. These files come in Moses format, i.e. **tab-separated** columns.

* `{lang1}-{lang2}.raw.gz`: parallel corpus that contains every aligned sentences, this file has **no deduplication** and the sentences are **not filtered**.

    The columns in this file are `url1 url2 sent1 sent2 aligner_score`, where:

    1. `url1` and `url2` correspond to the source documents of the sentences
    2. `sent1` and `sent2` correspond to the sentence pair in `lang1` and `lang2`
    3. `aligner_score` is the score given to the pair by the sentence aligner (bleualign or hunalign)

    This file comes accompanied by the corresponding statistics file `{lang1}-{lang2}.stats.raw`, which provides information the size of the corpus in MB and in number of  tokens.

* `{lang1}-{lang2}.sent.gz`: parallel corpus after adding **deferred** checksums, running **Bifixer**, filtering using **Bicleaner** with the specified threshold, adding **ELRC** metrics, and **sorted** to have the duplicate or (near-duplicate) sentences together.

    In this case, the columns depend largely on the configuration of Bitextor, so the columns corresponding to the steps that were not enabled will be omitted in this file.

    The possible columns that may appear in this file are (in this order):

    1. `url1 url2 sent1 sent2 aligner_score` - default columns
    2. `deferred_checksum1 deferred_checksum2` - deferred checksums
        * may be used to reconstruct the original corpus using [Deferred crawling reconstructor](https://github.com/bitextor/deferred-crawling)
    3. bifixer: `bifixer_hash bifixer_score` - Bifixer output
        * `bifixer_hash` tags duplicate or near-duplicate sentences
        * `bifixer_score` rates quality of duplicate or near-duplicate sentences
    4. `bicleaner_score` - Bicleaner score
    5. `length_ratio num_tokens_src num_tokens_trg` - ELRC fields
        * `num_tokens_src` is the number of tokens in source sentence
        * `num_tokens_trg` is the number of tokens in target sentence
        * `length_ratio` is the ratio between the two (source divided by target)

## TMX

Bitextor may also generate a [TMX](https://en.wikipedia.org/wiki/Translation_Memory_eXchange) version of the corpus, if it is enabled in config via `tmx: true`. The generated file is:

* `{lang1}-{lang2}.not-deduped.tmx.gz` will have the same content as `{lang1}-{lang2}.sent.gz`, but in **TMX** format.

## Deduplication

Bitextor may also perform the deduplication of the generated corpus (enabled via `dedup: true`), and generate the following files that will contain only unique sentence pairs:

* `{lang1}-{lang2}.deduped.tmx.gz`: **deduplicated TMX** corpus, which will contain a list of URLs for the sentence pairs that were found in multiple websites.

* `{lang1}-{lang2}.deduped.txt.gz`: **deduplicated tab-separated** corpus, with the same columns as `{lang1}-{lang2}.sent.gz`; in this case each sentence will only contain one source document

    Corresponding statistics file `{lang1}-{lang2}.stats.deduped` will be generetaed and it will have information about the size of the corpus in MB, the number of unique sentence pairs and the number of tokens on each size.

## Biroamer

If Biroamer is enabled `biroamer: true`, Bitextor will also produce a ROAMed version of the corpus, according to Biroamer [config](CONFIG.md#post-processing). Biroamer will produce either of these files, depending on `tmx` and `dedup` parameters:

* `{lang1}-{lang2}.deduped.roamed.tmx.gz`: **ROAMed** version of the **deduplicated** corpus in **TMX** format, that will be generated if `dedup` is true
* `{lang1}-{lang2}.not-deduped.roamed.tmx.gz`: **ROAMed** version of the corpus without deduplication in **TMX** format, that will be generated if `tmx` is enabled, but not `dedup`
