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

import argparse

def cut(fd, fields, delimiter='\t', respect_original_sorting=False):
    header = next(fd).strip().split(delimiter)
    idxs = {}

    for field in fields:
        idxs[field] = header.index(field)

    sorted_fields = []

    if respect_original_sorting:
        for h in header:
            if h in fields:
                sorted_fields.append(h)
    else:
        sorted_fields = fields

    # Print header
    print(delimiter.join(sorted_fields))

    # Print values
    for line in fd:
        line = line.split(delimiter)
        line[-1] = line[-1][:-1]
        to_print = []

        for field in sorted_fields:
            to_print.append(line[idxs[field]])

        print(delimiter.join(to_print))

def parse_args():
    parser = argparse.ArgumentParser(description="It joins the sentences of the content from BASE64 documents and returns a new BASE64-encoded document",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--input', default='-', type=argparse.FileType('r'),
                        help="Input file. If '-' is provided, stdin will be used")
    parser.add_argument('-f', '--fields', required=True,
                        help="Fields to cut (separated by ',')")
    parser.add_argument('-d', '--delimiter', default='\t',
                        help="Delimiter")
    parser.add_argument('--respect-original-sorting', action='store_true',
                        help="Original sorting will be respected instead of the order of the provided fields")

    args = parser.parse_args()

    args.fields = args.fields.split(',')

    return args

if __name__ == '__main__':
    args = parse_args()

    cut(args.input, args.fields, delimiter=args.delimiter, respect_original_sorting=args.respect_original_sorting)
