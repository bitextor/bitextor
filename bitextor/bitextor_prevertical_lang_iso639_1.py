#!/usr/bin/env python3

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
import logging
import argparse
import contextlib

import langcodes

def main(args):
    if args.input[0] == "-":
        args.input = args.input[:1]

    for fn in args.input:
        temp_fn = f"{fn}.tmp"
        context = open(fn, "r") if fn != "-" else contextlib.nullcontext(sys.stdin)

        with context as fd:
            for idx, l in enumerate(fd):
                if l[:5] == "<doc ":
                    lang_str_idx = l.find("lang=\"")
                    lang_end_idx = l.find("\"", lang_str_idx + 6)

                    if (lang_str_idx == -1 or lang_end_idx == -1):
                        logging.warning("Line %d in file %s: could not gather the language", idx, fn)
                        sys.stdout.write(l)
                        continue

                    lang = l[lang_str_idx + 6:lang_end_idx]
                    lang_iso_639_1 = langcodes.find(lang)

                    l = f"{l[:lang_str_idx]}lang=\"{lang_iso_639_1}{l[lang_end_idx:]}"

                sys.stdout.write(l)

def parse_args():
    parser = argparse.ArgumentParser(
        description='Process prevertical format in order to change full language names to ISO 639-1.')

    parser.add_argument('--input', required=True, nargs='+',
                        help='Input files (prevertical)')

    args = parser.parse_args()

    return args

if __name__ == "__main__":
    args = parse_args()
    
    main(args)