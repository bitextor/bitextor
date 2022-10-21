#  This file is part of Bitextor.
#
#  Bitextor is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bitextor is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bitextor.  If not, see <https://www.gnu.org/licenses/>.

import sys
import os
import pprint

from cerberus import Validator

from bitextor.utils.common import path_exists

def generic_error(msg):
    def f(field, value, error):
        error(field, msg)

    return f

def isfile(field, value, error):
    if isinstance(value, list):
        for element in value:
            if not path_exists(element):
                error(field, f'{element} does not exist')
    elif isinstance(value, dict):
        for element in value:
            if not path_exists(value[element]):
                error(field, f'{value[element]} does not exist')
    elif not path_exists(value):
        error(field, f'{value} does not exist')


def isstrlist(field, value, error):
    if not isinstance(value, list):
        error(field, f'{value} should be a list')
    for element in value:
        if not isinstance(element, str):
            error(field, f'{element} should be an string')


def isduration(field, value, error):
    if len(value) == 0:
        return False
    suffix_correct = value[-1].isalpha()
    prefix_correct = sum(not x.isnumeric() for x in value[:-1]) == 0
    if not suffix_correct or not prefix_correct:
        error(field, f"format is an integer followed by a single letter suffix")


def istrue(field, value, error):
    if value != True:
        error(field, f'{value} is not True')

def check_generate_dic(schema, provided_in_config, config, dic_required=True):
    if dic_required:
        schema['dic']['required'] = True

    if provided_in_config['dic'] and not path_exists(config['dic']):
        schema['generateDic']['required'] = True
        schema['generateDic']['check_with'] = istrue

def check_granularity(field, value, error):
    if not isinstance(value, list):
        error(field, f'{value} should be a list')
    if len(value)==1 and value[0]=="documents":
        error(field, "The option 'documents' requires the option 'sentences'")

def validate_args(config):
    schema = {
        # output folders
        'dataDir': {'type': 'string', 'required': True},
        'permanentDir': {'type': 'string', 'required': True},
        'transientDir': {'type': 'string', 'required': True},
        'tempDir': {
            'type': 'string',
            'default_setter': lambda doc: doc["transientDir"] if "transientDir" in doc else ""
        },
        # output files
        'granularity': {'type': 'list', 'check_with': check_granularity, 'allowed': ['sentences', 'documents'],  'default': ['sentences']},
        # profiling
        'profiling': {'type': 'boolean', 'default': False},
        # execute until X:
        'until': {
            'type': 'string',
            'allowed': [
                'crawl', 'preprocess', 'shard', 'split', 'translate', 'tokenise',
                'tokenise_src', 'tokenise_trg', 'docalign', 'segalign', 'filter'
            ],
            'meta': 'Stop the workflow at some specific point instead of waiting to finish'
        },
        'parallelWorkers': {
            'type': 'dict',
            'allowed': [
                'split', 'translate', 'tokenise', 'docalign', 'segalign', 'filter', 'sents', 'mgiza'
            ],
            'valuesrules': {'type': 'integer', 'min': 1},
            'meta': 'Parallelization configuration of the tools used in the pipeline (e.g. threads)',
        },
        'parallelJobs': {
            'type': 'dict',
            'allowed': [
                'split', 'translate', 'tokenise', 'docalign', 'segalign', 'bifixer', 'bicleaner'
            ],
            'valuesrules': {'type': 'integer', 'min': 1},
            'meta': 'Max. snakemake parallel jobs running at once',
        },
        # verbose
        'verbose': {'type': 'boolean', 'default': False},
        # data definition
        # TODO: check that one of these is specified?
        'hosts': {'type': 'list', 'dependencies': 'crawler'},
        'hostsFile': {'type': 'string', 'dependencies': 'crawler', 'check_with': isfile},
        'warcs': {'type': 'list', 'check_with': isfile},
        'warcsFile': {'type': 'string', 'check_with': isfile},
        'preverticals': {'type': 'list', 'check_with': isfile},
        'preverticalsFile': {'type': 'string', 'check_with': isfile},
        # crawling
        'crawler': {'type': 'string', 'allowed': ["wget", "heritrix", "linguacrawl"]},
        'crawlTimeLimit': {
            'type': 'string', 'dependencies': 'crawler',
            'check_with': isduration
        },
        ## wget or linguacrawl:
        'crawlerUserAgent': {'type': 'string', 'dependencies': {'crawler': ['wget', 'linguacrawl']}},
        'crawlWait': {'type': 'integer', 'dependencies': {'crawler': ['wget', 'linguacrawl']}},
        'crawlFileTypes': {'type': 'list', 'dependencies': {'crawler': ['wget', 'linguacrawl']}},
        ## only linguacrawl:
        'crawlTLD': {'type': 'list', 'check_with': isstrlist, 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlSizeLimit': {'type': 'integer', 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlCat': {'type': 'boolean', 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlCatMaxSize': {'type': 'integer', 'dependencies': {'crawlCat': True}},
        'crawlMaxFolderTreeDepth': {'type': 'string', 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlScoutSteps': {'type': 'string', 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlBlackListURL': {'type': 'list', 'check_with': isstrlist, 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlPrefixFilter': {'type': 'list', 'check_with': isstrlist, 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlerNumThreads': {'type': 'integer', 'dependencies': {'crawler': 'linguacrawl'}},
        'crawlerConnectionTimeout': {'type': 'integer', 'dependencies': {'crawler': 'linguacrawl'}},
        'dumpCurrentCrawl': {'type': 'boolean', 'dependencies': {'crawler': 'linguacrawl'}},
        'resumePreviousCrawl': {'type': 'boolean', 'dependencies': {'crawler': 'linguacrawl'}},
        ## only heritrix
        'heritrixPath': {'type': 'string', 'dependencies': {'crawler': 'heritrix'}},
        'heritrixUrl': {'type': 'string', 'dependencies': {'crawler': 'heritrix'}},
        'heritrixUser': {'type': 'string', 'dependencies': {'crawler': 'heritrix'}},
        # preprocessing
        'preprocessor': {'type': 'string', 'allowed': ['warc2text', 'warc2preprocess'], 'default': 'warc2text'},
        'langs': {'type': 'list'},
        'shards': {'type': 'integer', 'min': 0, 'default': 8},
        'batches': {'type': 'integer', 'min': 1, 'default': 1024},
        'paragraphIdentification': {'type': 'boolean', 'default': False},
        ## specific to warc2text:
        'writeHTML': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2text'}},
        ## specific to warc2preprocess:
        'cleanHTML': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
        'ftfy': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
        'langID': {
            'type': 'string',
            'allowed': ['cld2', 'cld3'],
            'dependencies': {'preprocessor': 'warc2preprocess'}
        },
        'parser': {
            'type': 'string',
            'allowed': ['bs4', 'modest', 'simple', 'lxml'],
            'dependencies': {'preprocessor': 'warc2preprocess'}
        },
        'html5lib': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
        ## pdfEXTRACT
        'PDFextract': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
        'PDFextract_configfile': {'type': 'string', 'dependencies': 'PDFextract'},
        'PDFextract_sentence_join_path': {'type': 'string', 'dependencies': 'PDFextract'},
        'PDFextract_kenlm_path': {'type': 'string', 'dependencies': 'PDFextract'},
        ## boilerplate (prevertical2text, i.e. preverticals, and warc2preprocess)
        'boilerplateCleaning': {'type': 'boolean', 'default': False},
        'boilerpipeMaxHeapSize': {'type': 'integer', 'dependencies': {'boilerplateCleaning': True, 'preprocessor': 'warc2preprocess'}},
        # tokenization
        'sentenceSplitters': {'type': 'dict'},
        'customNBPs': {'type': 'dict'},
        'wordTokenizers': {'type': 'dict'},
        'morphologicalAnalysers': {'type': 'dict'},
        'pruneThreshold': {'type': 'integer', 'min': 0, 'default': 0},
        'pruneType': {'type': 'string', 'allowed': ['words', 'chars'], 'default': 'words'},
        # document alignment
        'lang1': {'type': 'string'},
        'lang2': {'type': 'string'},
        'documentAligner': {
            'type': 'string',
            'allowed': ['DIC', 'externalMT', 'NDA'],
            'default': 'externalMT',
            'dependencies': {}
        },
        'documentAlignerThreshold': {'type': 'float'},
        # embeddings
        'embeddingsBatchSize': {'type': 'integer', 'min': 1, 'default': 32},
        'embeddingsModel': {'type': 'string', 'dependencies': {}},
        ## mt
        'alignerCmd': {'type': 'string', 'dependencies': {'documentAligner': 'externalMT'}},
        'translationDirection': {'type': 'string', 'dependencies': {'documentAligner': 'externalMT'}},
        ## dictionary
        'dic': {'type': 'string', 'dependencies': {}},
        'generateDic': {'type': 'boolean', 'default': False, 'dependencies': {}},
        'initCorpusTrainingPrefix': {'type': 'list'},
        # sentence alignment
        'sentenceAligner': {'type': 'string', 'allowed': ['bleualign', 'hunalign', 'vecalign'], 'default': 'bleualign'},
        'sentenceAlignerThreshold': {'type': 'float'},
        # post processing
        'deferred': {'type': 'boolean', 'default': False},
        ## fix
        'bifixer': {'type': 'boolean', 'default': False},
        ### mark near duplicates as duplicates
        'bifixerAggressiveDedup': {'type': 'boolean', 'dependencies': {'bifixer': True}},
        ### do not resplit
        'bifixerIgnoreSegmentation': {'type': 'boolean', 'dependencies': {'bifixer': True}},
        ## cleaning
        'bicleaner': {'type': 'boolean', 'default': False},
        'bicleanerFlavour': {'type': 'string', 'allowed': ['classic', 'ai'], 'default': 'ai'},
        'bicleanerModel': {'type': 'string', 'dependencies': {'bicleaner': True}},
        'bicleanerGenerateModel': {'type': 'boolean', 'default': False},
        'bicleanerThreshold': {'type': 'float'},
        'bicleanerParallelCorpusTrainingPrefix': {'type': 'list'},
        ### bicleaner AI
        'bicleanerMonoCorpusPrefix': {'type': 'list'},
        'bicleanerParallelCorpusDevPrefix': {'type': 'list'},
        ## elrc metrics
        'elrc': {'type': 'boolean', 'default': False},
        ## tmx
        'tmx': {'type': 'boolean', 'default': False},
        'deduped': {'type': 'boolean', 'default': False},
        ## roam
        'biroamer': {'type': 'boolean', 'dependencies': {'tmx': True}},
        'biroamerOmitRandomSentences': {'type': 'boolean', 'dependencies': {'biroamer': True}},
        'biroamerMixFiles': {'type': 'list', 'check_with': isfile, 'dependencies': {'biroamer': True}},
        'biroamerImproveAlignmentCorpus': {'type': 'string', 'check_with': isfile, 'dependencies': {'biroamer': True}},
    }

    provided_in_config = {} # contains info about the definition of rules in the configuration file
    monolingual_workflow = False

    # initialize with the default values if no value was provided
    for key in schema:
        provided_in_config[key] = True if key in config else False

        if key not in config and 'default' in schema[key]:
            config[key] = schema[key]['default']

    # cast dict values str to int
    for key in ("parallelWorkers", "parallelJobs"):
        if key not in config:
            continue

        try:
            for k, v in config[key].items():
                config[key][k] = int(v)
        except ValueError as e:
            generic_error(f"could not cast str to int: '{key}': {e}")

    both_langs_specified = provided_in_config["lang1"] and provided_in_config["lang2"]

    if provided_in_config['crawler'] and config['crawler'] == 'heritrix':
        schema['heritrixPath']['required'] = True

    if provided_in_config['until'] and config['until'] in ('crawl', 'preprocess', 'shard', 'split', 'tokenise'):
        monolingual_workflow = True

    if not monolingual_workflow:
        schema['lang1']['required'] = True
        schema['lang2']['required'] = True

    elif monolingual_workflow and not both_langs_specified:
        schema['langs']['required'] = True

    if config['boilerplateCleaning'] and config['preprocessor'] != 'warc2preprocess':
        if not provided_in_config['preverticals'] and not provided_in_config['preverticalsFile']:
            schema['boilerplateCleaning']['check_with'] = \
                generic_error("mandatory: preprocessor 'warc2preprocess' or provide prevertical files")

    if config['preprocessor'] == 'warc2preprocess':
        if provided_in_config['preverticals'] or provided_in_config['preverticalsFile']:
            schema['preverticals']['dependencies'] = {'preprocessor': ['warc2text']}
            schema['preverticalsFile']['dependencies'] = {'preprocessor': ['warc2text']}

    if config['documentAligner'] == 'externalMT':
        schema['alignerCmd']['required'] = True

        if monolingual_workflow and not provided_in_config['alignedCmd']:
            # document aligner is not going to be used, so fake value is provided
            config["alignerCmd"] = ""

        if both_langs_specified:
            schema['translationDirection']['allowed'] = [
                f'{config["lang1"]}2{config["lang2"]}',
                f'{config["lang2"]}2{config["lang1"]}']

        if config["sentenceAligner"] == "hunalign":
            check_generate_dic(schema, provided_in_config, config)

    elif config['documentAligner'] == 'DIC':
        schema['documentAligner']['dependencies']['preprocessor'] = ['warc2preprocess', 'warc2text']

        if config['preprocessor'] == 'warc2text':
            config['writeHTML'] = True

        check_generate_dic(schema, provided_in_config, config)

    if config['generateDic']:
        schema['dic']['required'] = True
        schema['initCorpusTrainingPrefix']['required'] = True

    if config['sentenceAligner'] == 'bleualign':
        schema['sentenceAligner']['dependencies'] = {'documentAligner': 'externalMT'}

    elif config['sentenceAligner'] == 'vecalign':
        schema['sentenceAligner']['dependencies'] = {'documentAligner': 'NDA'}

    if config['sentenceAligner'] != 'vecalign':
        schema['embeddingsModel']['dependencies']['sentenceAligner'] = 'vecalign'

    if config['documentAligner'] != 'NDA':
        schema['embeddingsModel']['dependencies']['documentAligner'] = 'NDA'

    if provided_in_config['deferred']:
        schema['until']['allowed'].append('deferred')

    if provided_in_config['bifixer']:
        schema['until']['allowed'].append('bifixer')
        schema['parallelWorkers']['allowed'].append('bifixer')
        schema['parallelJobs']['allowed'].append('bifixer')

    if config['bicleaner']:
        schema['until']['allowed'].append('bicleaner')
        schema['parallelWorkers']['allowed'].append('bicleaner')
        schema['parallelJobs']['allowed'].append('bicleaner')
        schema['bicleanerModel']['required'] = True

        if config['bicleanerGenerateModel']:
            schema['bicleanerParallelCorpusTrainingPrefix']['required'] = True

            if config['bicleanerFlavour'] == "classic":
                schema['initCorpusTrainingPrefix']['required'] = True

                check_generate_dic(schema, provided_in_config, config)
            elif config['bicleanerFlavour'] == "ai":
                schema['bicleanerMonoCorpusPrefix']['required'] = True
                schema['bicleanerParallelCorpusDevPrefix']['required'] = True
        else:
            schema['bicleanerModel']['check_with'] = isfile

    if provided_in_config['until'] and (config['until'] == 'filter' or config['until'] == 'bifixer'):
        print(
            "WARNING: your target consists of temporary files. Make sure to use --notemp parameter to preserve your output",
            file=sys.stderr)

    v = Validator(schema)
    b = v.validate(config)

    if not b:
        print("Validation errors. Stopping.", file=sys.stderr)
        pprint.pprint(v.errors, indent=2, stream=sys.stderr, width=100)
        return b, {}

    config.update({k: os.path.expanduser(v) if isinstance(v, str) else v for k, v in config.items()})
    config.update({k: [os.path.expanduser(i) for i in v] if v is list else v for k, v in config.items()})

    return b, v.normalized(config)
