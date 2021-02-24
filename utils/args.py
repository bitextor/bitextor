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

from cerberus import Validator

def isfile(field, value, error):
    if isinstance(value, list):
        for element in value:
            if not os.path.isfile(os.path.expanduser(element)):
                error(field, f'{element} does not exist')
    elif not os.path.isfile(os.path.expanduser(value)):
        error(field, f'{value} does not exist')

def isstrlist(field, value, error):
    if not isinstance(value, list):
        error(field, f'{value} should be a list')
    for element in value:
        if not isinstance(element, str):
            error(field, f'{element} should be an string')

def validate_args(config):
    schema = {
            # required parameters
            'bitextor': {'required': True, 'type': 'string'},
            # output folders
            'dataDir': {'type': 'string', 'required': True},
            'permanentDir': {'type': 'string', 'required': True},
            'transientDir': {'type': 'string', 'required': True},
            'tempDir': {'type': 'string', 'default_setter': lambda doc: doc["transientDir"]},
            # profiling
            'profiling': {'type': 'boolean', 'default': False},
            # execute until X:
            'until': {'type': 'string', 'allowed': ['crawl', 'preprocess', 'shard', 'split', 'translate', 'tokenise_src', 'tokenise_trg', 'docalign', 'segalign', 'filter']},
            'parallelWorkers': {'type': 'dict', 'allowed': ['split', 'translate', 'tokenise_src', 'tokenise_trg', 'docalign', 'segalign', 'sents'], 'valuesrules': {'type': 'integer', 'min': 1}},
            # data definition
            # TODO: check that one of these is specified?
            'hosts': {'type': 'list', 'dependencies': 'crawler'},
            'hostsFile': {'type': 'string', 'dependencies': 'crawler', 'check_with': isfile},
            'warcs': {'type': 'list', 'check_with': isfile},
            'warcsFile': {'type': 'string', 'check_with': isfile},
            # crawling
            'crawler': {'type': 'string', 'allowed': ["wget", "heritrix", "creepy", "httrack", "linguacrawl"]},
            'crawlTimeLimit': {'type': 'string', 'dependencies': 'crawler'},
            'crawlerUserAgent': {'type': 'string', 'dependencies': {'cralwer' : ['creepy', 'wget', 'httrack', 'linguacrawl']}},
            'crawlWait': {'type': 'string', 'dependencies': {'crawler': ['creepy', 'wget', 'httrack', 'linguacrawl']}},
            'crawlPageLimit': {'type': 'string', 'dependencies': {'crawler' : 'httrack'}},
            'crawlFileTypes': {'type': 'string', 'dependencies': {'crawler' : ['wget', 'linguacrawl']}},
            'crawlTLD': {'type': 'boolean', 'dependencies': {'crawler' : ['creepy', 'linguacrawl']}},
            'crawlSizeLimit': {'type': 'string', 'dependencies': {'crawler' : ['creepy', 'linguacrawl']}},
            'crawlCat': {'type': 'boolean', 'dependencies': {'crawler': 'linguacrawl'}},
            'crawlCatMaxSize': {'type': 'integer', 'dependencies': {'crawlCat': True}},
            'crawlMaxFolderTreeDepth': {'type': 'string', 'dependencies': {'crawler': 'linguacrawl'}},
            'crawlScoutSteps': {'type': 'string', 'dependencies': {'crawler': 'linguacrawl'}},
            'crawlBlackListURL': {'type': 'list', 'check_with': isstrlist, 'dependencies': {'crawler': 'linguacrawl'}},
            'crawlPrefixFilter': {'type': 'list', 'check_with': isstrlist, 'dependencies': {'crawler': 'linguacrawl'}},
            'crawlerNumThreads': {'type': 'string', 'dependencies': {'crawler' : ['creepy', 'linguacrawl']}},
            'crawlerConnectionTimeout': {'type': 'string', 'dependencies': {'crawler' : ['creepy', 'linguacrawl']}},
            'dumpCurrentCrawl': {'type': 'string', 'dependencies': {'crawler' : ['creepy', 'linguacrawl']}},
            'resumePreviousCrawl': {'type': 'string', 'dependencies': {'crawler' : ['creepy', 'linguacrawl']}},
            'heritrixPath': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
            'heritrixUrl': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
            'heritrixUser': {'type': 'string', 'dependencies': {'crawler' : 'heritrix'}},
            # preprocessing
            'preprocessor': {'type': 'string', 'allowed': ['warc2text', 'warc2preprocess', 'giawarc'], 'default': 'warc2text'},
            'langs': {'type': 'list'},
            'shards': {'type': 'integer', 'min': 0, 'default': 8},
            'batches': {'type': 'integer', 'min': 1, 'default': 1024},
            ## specific to warc2text:
            'writeHTML': {'type': 'boolean', 'dependencies': {'preprocessor':  ['warc2text']}},
            ## specific to warc2preprocess:
            'cleanHTML': {'type': 'boolean', 'default': False, 'dependencies': {'preprocessor': 'warc2preprocess'}},
            'ftfy': {'type': 'boolean', 'default': False, 'dependencies': {'preprocessor': 'warc2preprocess'}},
            'langID': {'type': 'string', 'allowed': ['cld2', 'cld3'], 'default': 'cld2'},
            'parser': {'type': 'string', 'allowed': ['bs4', 'modest', 'simple', 'lxml'], 'dependencies': {'preprocessor': 'warc2preprocess'}},
            'boilerpipeCleaning': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
            'html5lib': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
            ## pdfEXTRACT
            'PDFextract': {'type': 'boolean', 'dependencies': {'preprocessor': 'warc2preprocess'}},
            'PDFextract_configfile': {'type': 'string', 'dependencies': 'PDFextract'},
            'PDFextract_sentence_join_path': {'type': 'string', 'dependencies': 'PDFextract'},
            'PDFextract_kenlm_path': {'type': 'string', 'dependencies': 'PDFextract'},
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
            'documentAligner': {'type': 'string', 'allowed': ['DIC', 'externalMT'], 'default': 'externalMT', 'dependencies': {}},
            # mt
            'alignerCmd': {'type': 'string', 'dependencies': {'documentAligner': 'externalMT'}},
            'translationDirection': {'type': 'string', 'dependencies': {'documentAligner': 'externalMT'}},
            'documentAlignerThreshold': {'type': 'float', 'dependencies': {'documentAligner': 'externalMT'}},
            # dictionary
            'dic': {'type': 'string', 'dependencies': {}},
            'initCorpusTrainingPrefix': {'type': 'list'},
            'mkcls': {'type': 'boolean'},
            # sentence alignment
            'sentenceAligner': {'type': 'string', 'allowed': ['bleualign', 'hunalign'], 'default': 'bleualign'},
            'sentenceAlignerThreshold': {'type': 'float'},
            # post processing
            'deferred': {'type': 'boolean', 'default': False},
            'bifixer': {'type': 'boolean', 'default': False},
            'aggressiveDedup': {'type': 'boolean', 'dependencies': {'bifixer': True}}, # mark near duplicates as duplicates
            'bicleaner': {'type': 'string'},
            'bicleanerThreshold': {'type': 'float', 'dependencies': 'bicleaner'},
            'bicleanerCorpusTrainingPrefix': {'type': 'list'},
            'elrc': {'type': 'boolean'},
            'tmx': {'type': 'boolean'},
            'deduped': {'type': 'boolean'},
            'biroamer': {'type': 'boolean', 'default': False},
            'biroamerOmitRandomSentences': {'type': 'boolean', 'dependencies': {'biroamer': True}},
            'biroamerMixFiles': {'type': 'list', 'check_with': isfile, 'dependencies': {'biroamer': True}},
            'biroamerImproveAlignmentCorpus': {'type': 'string', 'check_with': isfile, 'dependencies': {'biroamer': True}}
            }

    if 'crawler' in config:
        if config['crawler'] == 'heritrix':
            schema['heritrixPath']['required'] = True
        elif config['crawler'] == 'linguacrawl':
            schema['dumpCurrentCrawl']['type'] = 'boolean'
            schema['crawlTLD']['type'] = 'list'
            schema['crawlTLD']['check_with'] = isstrlist
            schema['resumePreviousCrawl']['type'] = 'boolean'
    
    if ('onlyPreprocess' not in config or not config['onlyPreprocess']) and ('onlyCrawl' not in config or not config['onlyCrawl']):
        schema['lang1']['required'] = True
        schema['lang2']['required'] = True

    elif ('onlyPreprocess' in config and config['onlyPreprocess']) and ('lang1' not in config or 'lang2' not in config):
        # if onlyPreprocess in true, target languages should be indicated either with 'lang1' and 'lang2', or 'langs'
        schema['langs']['required'] = True
    
    if "documentAligner" not in config or config['documentAligner'] == 'externalMT':
        schema['alignerCmd']['required'] = True
        schema['translationDirection']['allowed'] = [f'{config["lang1"]}2{config["lang2"]}', f'{config["lang2"]}2{config["lang1"]}']

        if "sentenceAligner" in config and config["sentenceAligner"] == "hunalign":
            schema['dic']['required'] = True
    elif config['documentAligner'] == 'DIC':
        schema['dic']['required'] = True
        schema['documentAligner']['dependencies']['preprocessor'] = ['warc2preprocess', 'warc2text']
        if config['preprocessor'] == 'warc2text':
            config['writeHTML'] = True

    if "sentenceAligner" not in config or config['sentenceAligner'] == 'bleualign':
        #schema['sentenceAligner']['dependencies'] = frozenset({'documentAligner': 'externalMT'}) # dependencies are not working because of the frozenset
        schema['sentenceAligner']['dependencies'] = {'documentAligner': 'externalMT'}

    if "deferred" in config:
        schema['until']['allowed'].append('deferred')
        schema['parallelWorkers']['allowed'].append('deferred')

    if 'bifixer' in config:
        schema['until']['allowed'].append('bifixer')
        schema['parallelWorkers']['allowed'].append('bifixer')

    if 'bicleaner' in config:
        schema['until']['allowed'].append('bicleaner')
        schema['parallelWorkers']['allowed'].append('bicleaner')
        
        if not os.path.isfile(os.path.expanduser(config['bicleaner'])):
            schema['bicleanerCorpusTrainingPrefix']['required']=True
            schema['initCorpusTrainingPrefix']['required']=True
            schema['dic']['required'] = True

    if 'dic' in config and not os.path.isfile(os.path.expanduser(config['dic'])):
        # if 'dic' in config and does not exist, we need to generate a new dictionary
        schema['initCorpusTrainingPrefix']['required']=True

    if 'initCorpusTrainingPrefix' in config:
        schema['dic']['required'] = True

    if 'until' in config and (config['until'] == 'filter' or config['until'] == 'bifixer'):
        sys.stderr.write("WARNING: your target consists of temporary files. Make sure to use --notemp parameter to preserve your output\n")

    if 'biroamer' in config and config['biroamer']:
        if ('tmx' not in config or not config['tmx']) and ('deduped' not in config or not config['deduped']):
            sys.stderr.write("ERROR: if you want to use biroamer, you need either 'tmx' or 'deduped' config option set to 'true' (if both, deduped will be used)\n")

            if 'deduped' not in config:
                # tmx not in config or tmx in config but false
                schema['biroamer']['dependencies'] = {'tmx': True}
            else:
                # deduped in config but false and (tmx not in config or tmx in config and false)
                # debuped as default value in both situations
                schema['biroamer']['dependencies'] = {'deduped': True}

    v = Validator(schema)
    b = v.validate(config)

    if not b:
        print("Validation error. Stopping.", v.errors, file=sys.stderr)
        exit()

    config.update({k: os.path.expanduser(v) if isinstance(v, str) else v for k, v in config.items()}) 
    config.update({k: [os.path.expanduser(i) for i in v] if v is list else v for k, v in config.items()})

    return v.normalized(config)
