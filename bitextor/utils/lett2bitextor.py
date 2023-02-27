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

import os
import argparse
import logging
import gzip
import stat
import sys
import contextlib

from tldextract import extract

from common import open_xz_or_gzip_or_plain

def main(args):
    sys.stdin.reconfigure(encoding='utf-8', errors="backslashreplace")

    lett_file = args.lett_file
    langs = args.langs
    output_prefix = args.output_prefix
    preprocess_tool = args.preprocess_tool

    if lett_file != '-' and not os.path.isfile(lett_file) and not stat.S_ISFIFO(os.stat(lett_file).st_mode):
        raise Exception("LETT file is not a file")
    if not os.path.isdir(output_prefix):
        raise Exception("Output prefix is not a dir")

    output_files = {}
    number_output_files_open = {}
    preprocess_files = ("text", "url", "mime", "html")
    lett_context = gzip.open(lett_file, 'rt') if lett_file != '-' else contextlib.nullcontext(enter_result=sys.stdin)

    with lett_context as lett:
        for idx, line in enumerate(lett, 1):
            line = line.rstrip('\n').split('\t')

            if len(line) != 6:
                logging.warning("Line %d: unexpected number of columns: %d vs 6", idx, len(line))

            lang, mime, encoding, url, base64_html, base64_text = line
            tsd, td, tsu = extract(url)

            if len(langs) == 0 or lang in langs:
                # Write record

                open_files = lang not in output_files

                # Create necessary idx per domain
                if lang not in number_output_files_open:
                    number_output_files_open[lang] = {}
                    number_output_files_open[lang][td] = 0
                    open_files = True
                elif td not in number_output_files_open[lang]:
                    number_output_files_open[lang][td] = len(number_output_files_open[lang])
                    open_files = True

                if open_files:
                    prefix = f"{output_prefix}/{number_output_files_open[lang][td]}/{preprocess_tool}/{lang}"

                    if not os.path.isdir(prefix):
                        os.makedirs(prefix)

                    if lang not in output_files:
                        output_files[lang] = {}

                    # Open files
                    output_files[lang][td] = {f: gzip.open(f"{prefix}/{f}.gz", "wb") for f in preprocess_files}

                # Write
                output_files[lang][td]["text"].write(base64_text.encode("utf-8"))
                output_files[lang][td]["url"].write(url.encode("utf-8"))
                output_files[lang][td]["mime"].write(mime.encode("utf-8"))
                output_files[lang][td]["html"].write(base64_html.encode("utf-8"))

                for preprocess_file in preprocess_files:
                    output_files[lang][td][preprocess_file].write(b'\n')

    # Close files
    for lang in output_files:
        for td in output_files[lang]:
            for ft in output_files[lang][td]:
                output_files[lang][td][ft].close()

def parse_args():
    parser = argparse.ArgumentParser(description="Create necessary files from a LETT filke in order to be able to be processed by Bitextor",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--lett-file', default='-',
                        help="Input file")
    parser.add_argument('--langs', nargs='*',
                        help="All langs that will be processed (if none, all langs will be processed)")
    parser.add_argument('--output-prefix', required=True,
                        help="Output prefix (/path/to/preprocess)")
    parser.add_argument('--preprocess-tool', default='warc2text',
                        help="Preprocess tool that will be used in Bitextor")

    args = parser.parse_args()

    return args

if __name__ == "__main__":
    args = parse_args()
    main(args)
