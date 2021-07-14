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
    options.append("-s")
    options.append(snakefile)
    snakemake.main(options)


if __name__ == '__main__':
    main()
