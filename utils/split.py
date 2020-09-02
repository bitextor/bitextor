import argparse
import gzip
from sys import stdin

def columns(cols):
    ranges = []
    for c in cols.strip().split(','):
        start=1
        end=1
        if c.startswith('-'):
            start = None
            end = int(c)
        elif c.endswith('-'):
            start = int(c) - 1
            end = None 
        elif '-' in c:
            start = c.split('-')[0] - 1
            end = c.split('-')[1]
        else:
            start = int(c) - 1
            end = int(c)
        ranges.append((start, end))
    return ranges
        
oparser = argparse.ArgumentParser('split file by size of specified fields')
oparser.add_argument('-s', '--size', default=1024, help="size in MB")
oparser.add_argument('-o', '--output', default="", help="output files prefix")
oparser.add_argument('-f', '--fields', type=columns, default="3,4", help="relevant fields")
oparser.add_argument('--gzip', action="store_true", default=False, help="compress the output")
options = oparser.parse_args()

output_file = None
max_size = int(options.size) * 1024 * 1024
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
        file_n = file_n + 1
        current_size = 0
    for (start, end) in options.fields:
        current_size += len(' '.join(fields[start:end]))
    output_file.write(line)
