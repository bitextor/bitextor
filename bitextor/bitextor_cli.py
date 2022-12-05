#!/usr/bin/env python3

import argparse
import yaml
import snakemake
from os import path
from sys import argv


def parse_args():
    oparser = argparse.ArgumentParser(
        description="launch Bitextor", add_help=False)
    config = oparser.add_argument_group("Bitextor config")

    config.add_argument("-C", "--configfile", dest="configfiles", metavar="FILE",
                        help="Bitextor YAML configuration file", nargs='+')
    config.add_argument("-c", "--config", dest="config", metavar="KEY=VALUE", nargs="+", action=ParseDict,
                        help="Set or overwrite values for Bitextor config")

    optional = oparser.add_argument_group("Optional arguments")

    optional.add_argument("-j", "--jobs", dest="jobs", type=int, default=1,
                          help="Number of provided cores")

    optional.add_argument("-k", "--keep-going", dest="keep_going", action="store_true", default=False,
                          help="Go on with independent jobs if a job fails")
    optional.add_argument("--notemp", dest="notemp", action="store_true", default=False,
                          help="Disable deletion of intermediate files marked as temporary")
    optional.add_argument("--dry-run", dest="dry_run", action="store_true", default=False,
                          help="Do not execute anything and display what would be done")

    optional.add_argument("--forceall", dest="forceall", action="store_true", default=False,
                          help="Force rerun every job")
    optional.add_argument("--forcerun", dest="forcerun", metavar="TARGET", nargs="+",
                          help="List of files and rules that shall be re-created/re-executed")

    optional.add_argument("-q", "--quiet", dest="quiet", action="store_true", default=False,
                          help="Do not print job information")
    optional.add_argument("-h", "--help", action="help",
                          help="Show this help message and exit")

    options = oparser.parse_args()

    if not options.config and not options.configfiles:
        oparser.error("provide Bitextor configuration via --configfile or --config")

    return options


# parsing Bitextor config passed a list of KEY=VALUE pairs
class ParseDict(argparse.Action):

    @staticmethod
    def _yaml_base_load(s):
        return yaml.load(s, Loader=yaml.loader.BaseLoader)

    @staticmethod
    def _bool_parser(value):
        if value == "True":
            return True
        elif value == "False":
            return False
        raise ValueError

    @staticmethod
    def parse_value(value):
        for parser in [int, float, ParseDict._bool_parser, ParseDict._yaml_base_load, str]:
            try:
                return parser(value)
            except BaseException:
                pass

    def __call__(self, parser, namespace, values, option_string=None):
        d = getattr(namespace, self.dest) or {}

        if values:
            for item in values:
                parts = item.split('=', 1)
                k = parts[0].strip()
                v = ParseDict.parse_value(parts[1])
                d[k] = v

        setattr(namespace, self.dest, d)


def get_snakefile():
    return path.join(path.dirname(path.abspath(__file__)), "Snakefile")


def main():
    snakefile = get_snakefile()
    options = parse_args()

    snakemake.snakemake(
        snakefile=snakefile,
        configfiles=options.configfiles,
        config=options.config,
        nodes=options.jobs,
        cores=options.jobs,
        keepgoing=options.keep_going,
        notemp=options.notemp,
        dryrun=options.dry_run,
        quiet=options.quiet,
        forceall=options.forceall,
        forcerun=options.forcerun
    )


def main_full():
    snakefile = get_snakefile()
    options = argv[1:]
    options.append("--rerun-trigger") # Temporary, until we understand what is making Snakemake >= 7.8 re-run warc2preprocess step
    options.append("mtime") # This line too. See pinned issue/announcement https://github.com/snakemake/snakemake/issues/1694
    options.append("-s")
    options.append(snakefile)
    snakemake.main(options)

def wizard_configfile():
    from bitextor.utils.args import validate_args
    import json

    config = {}

    configfilename = ""

    if len(argv) > 1:
        configfilename = argv[1]
    else:
        while not configfilename:
             configfilename = input("Enter the output config file path: ")
    config["hosts"] = []
    host = "123"
    while host:
        host=input("Enter a host to crawl; p.e. example.com, not http://example.com (press the Enter key if you don't want to crawl any more hosts): ")
        if host:
            if len(host)>4 and (host[:5] == "http:" or host[:6] == "https:"):
                print("ERROR: that is a URL, not a host")
                continue
            else:
                config["hosts"].append(host)
        print("Queued hosts: " + str(config["hosts"]))

    config["crawler"] = "wget"
    config["permanentDir"]=input("Enter a path to store final data (permanent folder) [~/permanent]: ")
    if not config["permanentDir"]:
        config["permanentDir"]="~/permanent"

    config["transientDir"]=input("Enter a path to store intermediate data (transient folder) [~/transient]: ")
    if not config["transientDir"]:
        config["transientDir"]="~/transient"

    config["dataDir"]=input("Enter a path to store WARC data (data folder) [~/data]: ")
    if not config["dataDir"]:
        config["dataDir"]="~/data"

    pair = ""
    while "-" not in pair:
        pair = input("Which language pair do you want to generate? [en-fr]: ")
    if not pair:
        pair = "en-fr"
    config["lang1"] = pair.split('-')[0].lower()
    config["lang2"] = pair.split('-')[1].lower()

    config["documentAligner"] = ""
    while not config["documentAligner"]:
        config["documentAligner"] = input("Do you want to use the dictionary based document aligner or the translation based document aligner? Write 'dic' or 'mt': ")
        if config["documentAligner"] == "dic":
            config["documentAligner"] = "DIC"
            config["dic"] = ""
            while not config["dic"]:
                config["dic"] = input("Enter the dictionary file path (there are some already available for download here https://github.com/bitextor/bitextor-data/releases/tag/bitextor-v1.0 ): ")
            if not path.isfile(config["dic"]):
                config["generateDic"] = True
                config["initCorpusTrainingPrefix"] = [input("WARNING: provided dictionary file does not exist. Please, enter a corpus to train one: ")]
                answer = "yes"
                while answer.lower() == "yes" or answer.lower() == "y":
                    answer = input("Do you want to use more corpora to train the dictionary?: ")
                    if answer.lower() == "yes" or answer.lower() == "y":
                        config["initCorpusTrainingPrefix"].append(input("Please, enter an additional training corpus: "))


            config["sentenceAligner"] = "hunalign"

        elif config["documentAligner"] == "mt":
            config["documentAligner"] = "externalMT"
            config["alignerCmd"] = input("Enter command line to translate documents for its alignment [cat -]: ")
            if not config["alignerCmd"]:
                config["alignerCmd"] = "cat -"
            config["sentenceAligner"] = "bleualign"
        else:
            config["documentAligner"] = ""

    config["bicleaner"] = False
    answer = ""
    while not answer:
        answer = input("Do you want to use Bicleaner?: ")
        answer = answer.lower()
    if answer == "yes" or answer == "y":
        config["bicleaner"] = True
        config["bicleanerModel"] = ""
        while config["bicleanerModel"] == "":
            config["bicleanerModel"] = input("Provide a valid Bicleaner model path (you can download a pretrained one here https://github.com/bitextor/bicleaner-ai-data/releases/tag/v1.0 ): ")


    valid, config = validate_args(config)

    with open(configfilename, 'w') as fp:
        json.dump(config, fp, indent=0, ensure_ascii=False)
    print("File generated. Please, check https://github.com/bitextor/bitextor/blob/master/docs/CONFIG.md to add more advanced options and settings manually to the config file.")


if __name__ == '__main__':
    main()
