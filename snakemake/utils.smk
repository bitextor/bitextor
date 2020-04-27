import tldextract
import sys
import os
from itertools import product
from cerberus import Validator


def create_domain_key_2_host_map(hosts):
    key2hosts = {}
    for host in hosts:
        # don't merge blog sites
        if host.find(".blogspot.") >= 0 or host.find(".wordpress.") >= 0:
            key = host
        else:
            key = tldextract.extract(host).domain

        if key not in key2hosts:
            key2hosts[key] = []
        key2hosts[key].append(host)
    return key2hosts


def parent_folder_2_warcs(warcs):
    f2w = {}
    for warc in warcs:
        folder = warc.split('/')[-2]
        if folder not in f2w:
            f2w[folder] = []
        f2w[folder].append(warc)
    return f2w


def get_lang_or_default(scripts_dict, language):
    cmd = ""
    if language in scripts_dict:
        cmd = scripts_dict[language]
    elif "default" in scripts_dict:
        cmd = scripts_dict["default"]
    return cmd


def get_customnbp(nbp_dict, language):
    nbp = ""
    if language in nbp_dict:
        nbp = nbp_dict[language]
    return nbp


def get_mt_docalign_inputs(src_batches, trg_batches):
    # product( [[shard, batch], [shard, batch], ...], [[shard, batch], [shard, batch], ...] )
    iterator = product( [batch.split('/')[-2:] for batch in src_batches], [batch.split('/')[-2:] for batch in trg_batches] )
    # each item -> (shard, (src_batch, trg_batch))
    return [(src_shard, (src_batch, trg_batch)) for ((src_shard, src_batch), (trg_shard, trg_batch)) in iterator if src_shard == trg_shard]


def isfile(field, value, error):
    if not os.path.isfile(os.path.expanduser(value)):
        error(field, f'{value} does not exist')


def ispositive(field, value, error):
    if value <= 0:
        error(field, f'{value} should be greater than zero')


def ispositiveorzero(field, value, error):
    if value < 0:
        error(field, f'{value} should be greater than or equal to zero')    


def validate_args(config):
    schema = {
            # required parameters
            'bitextor': {'required': True, 'type': 'string'},
            # output folders
            'dataDir': {'type': 'string', 'required': True},
            'permanentDir': {'type': 'string', 'required': True},
            'transientDir': {'type': 'string', 'required': True},
            'tempDir': {'type': 'string'},
            # profiling
            'profiling': {'type': 'boolean'},
            # execute until X:
            'onlyCrawling': {'type': 'boolean'},
            'onlyPreprocess': {'type': 'boolean'},
            # data definition
            # TODO: check that one of these is specified?
            'hosts': {'type': 'list', 'dependencies': 'crawler'},
            'hostsFile': {'type': 'string', 'dependencies': 'crawler', 'check_with': isfile},
            'warcs': {'type': 'list'},
            'warcsFile': {'type': 'string', 'check_with': isfile},
            # crawling
            'crawler': {'type': 'string', 'allowed': ["wget", "heritrix", "creepy", "httrack"]},
            'crawlTimeLimit': {'type': 'string', 'dependencies': 'crawler'},
            'crawlerUserAgent': {'type': 'string', 'dependencies': {'cralwer' : ['creepy', 'wget', 'httrack']}},
            'crawlWait': {'type': 'string', 'dependencies': {'crawler': ['creepy', 'wget', 'httrack']}},
            'crawlPageLimit': {'type': 'string', 'dependencies': {'crawler' : 'httrack'}},
            'crawlFileTypes': {'type': 'string', 'dependencies': {'crawler' : 'wget'}},
            'crawl-tld': {'type': 'boolean', 'dependencies': {'crawler' : 'creepy'}},
            'crawlSizeLimit': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
            'crawlerNumThreads': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
            'crawlerConnectionTimeout': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
            'dumpCurrentCrawl': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
            'resumePreviousCrawl': {'type': 'string', 'dependencies': {'crawler' : 'creepy'}},
            'heritrixPath': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
            'heritrixUrl': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
            'heritrixUser': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
            # preprocessing
            'langs': {'type': 'list'},
            'preprocessor': {'type': 'string', 'allowed': ['warc2preprocess', 'giawarc']},
            'shards': {'type': 'integer', 'check_with': ispositiveorzero},
            'batches': {'type': 'integer', 'check_with': ispositive},
            'cleanHTML': {'type': 'boolean'},
            'ftfy': {'type': 'boolean'},
            'PDFextract': {'type': 'boolean'},
            'langID': {'type': 'string', 'allowed': ['cld2', 'cld3']},
            'parser': {'type': 'string', 'allowed': ['alcazar', 'bs4', 'modest', 'simple'], 'dependencies': {'preprocessor': ['warc2preprocess', '']}},
            'boilerpipeCleaning': {'type': 'boolean', 'dependencies': {'preprocessor': ['warc2preprocess', '']}},
            # tokenization
            'sentenceSplitters': {'type': 'dict'},
            'customNBPs': {'type': 'dict'},
            'workTokenizers': {'type': 'dict'},
            'norphologicalAnalysers': {'type': 'dict'},
            'pruneThreshold': {'type': 'integer', 'check_with': ispositive},
            'pruneType': {'type': 'string', 'allowed': ['words', 'chars']},
            # document alignment
            'lang1': {'type': 'string'},
            'lang2': {'type': 'string'},
            'documentAligner': {'type': 'string', 'allowed': ['DIC', 'externalMT']},
            # mt
            'alignerCmd': {'type': 'string', 'dependencies': {'documentAligner': 'externalMT'}},
            'translationDirection': {'type': 'string', 'dependencies': {'documentAligner': 'externalMT'}},
            'documentAlignerWorkers': {'type': 'integer', 'check_with': ispositive, 'dependencies': {'documentAligner': 'externalMT'}}, # positive integer (using 'all' doesn't make sense in snakemake)
            'documentAlignerThreshold': {'type': 'float', 'dependencies': {'documentAligner': 'externalMT'}},
            # dictionary
            'dic': {'type': 'string', 'check_with': isfile}, # TODO: depends on documentAligner=DIC, or sentenceAligner=hunalign, TODO: check if dictionary exists, use training subworkflow if not
            # sentence alignment
            'sentenceAligner': {'type': 'string', 'allowed': ['bleualign', 'hunalign']},
            'sentenceAlignerWorkers': {'type': 'integer', 'check_with': ispositive},
            'sentenceAlignerThreshold': {'type': 'float'},
            # post processing
            'deferred': {'type': 'boolean'},
            'bifixer': {'type': 'boolean'},
            'aggressiveDedup': {'type': 'boolean', 'dependencies': {'bifixer': True}}, # mark near duplicates as duplicates
            'bicleaner': {'type': 'string', 'check_with': isfile}, # TODO: check that model exists, use training subworkflow if not
            'bicleanerThreshold': {'type': 'float', 'dependencies': 'bicleaner'},
            'elrc': {'type': 'boolean'},
            'tmx': {'type': 'boolean'},
            'deduped': {'type': 'boolean'}
            }

    if 'crawler' in config and config['crawler'] == 'heritrix':
        schema['heritrixPath']['required'] = True
    
    if ('onlyPreprocess' not in config or not config['onlyPreprocess']) and ('onlyCrawl' not in config or not config['onlyCrawl']):
        schema['lang1']['required'] = True
        schema['lang2']['required'] = True

    elif ('onlyPreprocess' in config and config['onlyPreprocess']) and ('lang1' not in config or 'lang2' not in config):
        # if onlyPreprocess in true, target languages should be indicated either with 'lang1' and 'lang2', or 'langs'
        schema['langs']['required'] = True
    
    if 'documentAligner' not in config or config['documentAligner'] == 'DIC':
        schema['dic']['required'] = True
        schema['documentAligner']['dependencies'] = frozenset({'preprocessor': ['warc2preprcess', '']}),
    elif config['documentAligner'] == 'externalMt':
        schema['alignerCmd']['required'] = True
        schema['translationDirection']['allowed'] = [f'{schema["lang1"]}2{schema["lang2"]}', f'{schema["lang2"]}2{schema["lang1"]}']

    if 'sentenceAligner' in config and config['sentenceAligner'] == 'bleualign':
        schema['sentenceAligner']['dependencies'] = frozenset({'documentAligner': 'externalMT'})

    v = Validator(schema)
    b = v.validate(config)

    if not b:
        print("Validation error. Stopping.", v.errors, file=sys.stderr)
        exit()

    config.update({k: os.path.expanduser(v) if isinstance(v, str) else v for k, v in config.items()}) 
    config.update({k: [os.path.expanduser(i) for i in v] if v is list else v for k, v in config.items()})
