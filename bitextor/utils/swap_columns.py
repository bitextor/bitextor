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

# Format: [[src_field, trg_field] ...]

import sys
import copy

if len(sys.argv[1:]) % 2 != 0:
    raise Exception("The number of provided arguments have to be even, not odd")

src_columns = []
trg_columns = []

header = next(sys.stdin).strip().split('\t')
arg_idx = 0

while arg_idx < len(sys.argv[1:]):
    src_arg = sys.argv[arg_idx + 1]
    trg_arg = sys.argv[arg_idx + 2]

    if src_arg not in header or trg_arg not in header:
        raise Exception(f"The provided arguments are not in the header: '{src_arg}' or/and '{trg_arg}'")

    src_columns.append(src_arg)
    trg_columns.append(trg_arg)

    arg_idx += 2

target_header = copy.copy(header)

for src_column, trg_column in zip(src_columns, trg_columns):
    src_idx = header.index(src_column)
    trg_idx = header.index(trg_column)

    aux = header[trg_idx]
    target_header[trg_idx] = header[src_idx]
    target_header[src_idx] = aux

print('\t'.join(target_header))

for idx, line in enumerate(sys.stdin):
    fields = line.split('\t')
    fields[-1] = fields[-1].strip()

    if len(fields) != len(header):
        raise Exception(f"The line {idx + 2} does not have the expected number of fields ({len(fields)} vs {len(header)})")

    d = {}

    for h, f in zip(header, fields):
        d[h] = f

    final_values = []

    for h in target_header:
        final_values.append(d[h])

    print('\t'.join(final_values))
