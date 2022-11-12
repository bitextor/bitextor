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

import base64
import argparse

def main(args):
    for l in args.input:
        nolines = base64.b64decode(l.rstrip('\n').encode()).decode().rstrip('\n').count('\n') + 1
        metadata = '\n'.join(map(lambda noline: (args.delimiter).join(
            ['' if args.empty_fields else f"{args.prefix}_field{i}_{noline}" for i in range(1, args.fields + 1)]
            ), range(1, nolines + 1))) + '\n'

        args.output.write(f"{base64.b64encode(metadata.encode()).decode()}\n")

def parse_args():
    parser = argparse.ArgumentParser(description="It joins the sentences of the content from BASE64 documents and returns a new BASE64-encoded document",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("--input", default='-', type=argparse.FileType('r'),
                        help="Input file")
    parser.add_argument("--output", default='-', type=argparse.FileType('w'),
                        help="Output file")
    parser.add_argument("-f", "--fields", type=int, default=1,
                        help="Number of fields")
    parser.add_argument("-d", "--delimiter", default='\t',
                        help="Delimiter")
    parser.add_argument("--prefix", default="fake_metadata",
                        help="Common prefix for the metadata fields")
    parser.add_argument("--empty-fields", action="store_true",
                        help="Print empty ")

    args = parser.parse_args()

    return args

if __name__ == '__main__':
    args = parse_args()

    main(args)
