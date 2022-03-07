import argparse
import gzip
from sys import stdin

def columns(cols, header):
    ranges = []
    h = header.strip().split('\t')

    for c in cols.strip().split(','):
        try:
            idx = h.index(c)

            ranges.append((idx, idx + 1))
        except ValueError as e:
            raise Exception(f"The provided header field '{c}' is not in the header") from e

    return ranges

header = next(stdin)

oparser = argparse.ArgumentParser('split file by size of specified fields')
oparser.add_argument('-s', '--size', default=1024, help="size in MB")
oparser.add_argument('-o', '--output', default="", help="output files prefix")
oparser.add_argument('-f', '--fields', type=lambda c: columns(c, header), default="src_text,trg_text", help="relevant fields")
oparser.add_argument('--gzip', action="store_true", default=False, help="compress the output")
options = oparser.parse_args()

output_file = None
max_size = int(options.size) * 1024 * 1024 # Bytes
file_n = 0
current_size = max_size

for line in stdin:
    fields = line.strip().split('\t')

    if current_size >= max_size:
        if output_file is not None:
            output_file.close()
        if options.gzip:
            output_file = gzip.open(f'{options.output}{file_n}.gz', 'tw')
        else:
            output_file = open(f'{options.output}{file_n}', 'w')

        output_file.write(header)

        file_n = file_n + 1
        current_size = 0

    for (start, end) in options.fields:
        current_size += len(' '.join(fields[start:end]))

    output_file.write(line)
