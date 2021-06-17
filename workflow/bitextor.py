#!/usr/bin/env python3

import argparse
import yaml
import snakemake
import os.path

def parse_args():
    oparser = argparse.ArgumentParser(description="lauch Bitextor", add_help=False)
    config = oparser.add_argument_group("Bitextor config:")

    config.add_argument("-C", "--configfile", dest="configfiles", metavar="FILE", help="Bitextor configuration file", nargs='+')
    config.add_argument("-c", "--config", dest="config", metavar="KEY=VALUE", nargs="*", action=ParseDict, help="Specify values for Bitextor config")

    optional = oparser.add_argument_group("Optional arguments:")

    optional.add_argument("-j", "--jobs", dest="jobs", type=int, help="Number of provided cores", default=1)
    optional.add_argument("-k", "--keep-going", dest="keep_going", help="Go on with independent jobs if a job fails", action="store_true", default=False)
    optional.add_argument("--notemp", dest="notemp", help="Ignore temp() declarations, i.e. disable deletion of intermediate files", action="store_true", default=False)
    optional.add_argument("--dry-run", dest="dry_run", help="Do not execute anything and display what would be done", action="store_true", default=False)
    optional.add_argument("-q", "--quiet", dest="quiet", help="Do not print job information", action="store_true", default=False)
    optional.add_argument("--forceall", dest="forceall", help="Force rerun every job", action="store_true", default=False)
    optional.add_argument("--forcerun", dest="forcerun", help="List of files and rules that shall be re-created/re-executed", nargs="*")
    optional.add_argument("-h", "--help", help="Show this help message and exit", action="help")

    options = oparser.parse_args()

    if not options.config and not options.configfiles:
        oparser.error("provide Bitextor configuration via --configfile or --config")

    print(options.config)

    return options

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
            except:
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

def main():
    snakefile = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Snakefile")
    options = parse_args()

    snakemake.snakemake(
        snakefile=snakefile,
        configfiles=options.configfiles,
        config=options.config,
        cores=options.jobs,
        keepgoing=options.keep_going,
        notemp=options.notemp,
        dryrun=options.dry_run,
        quiet=options.quiet,
        forceall=options.forceall,
        forcerun=options.forcerun
    )

if __name__ == '__main__':
    main()
