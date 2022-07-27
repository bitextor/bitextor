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

try:
    import lzma
except ImportError:
    from backports import lzma

import gzip
from contextlib import contextmanager

import subprocess
import sys
import os
import requests
import shlex

class ExternalTextProcessor(object):

    def __init__(self, cmd, raise_exception=True, return_debug_data=False):
        self.raise_exception = raise_exception
        self.return_debug_data = return_debug_data

        if isinstance(cmd, str):
            # Split the command as bash does
            #  This adds support to quotes (e.g. "echo 'hi, bye'" -> ["echo", "hi, bye"] -> "hi, bye")
            #  Without quotes support, if quotes are added, the command might be, and likely will be, incorrect
            #   (e.g. "echo 'hi, bye'" -> ["echo", "'hi,", "bye'"] -> "'hi, bye'")
            self.cmd = shlex.split(cmd)
        else:
            self.cmd = cmd

    def process(self, input_text):
        proc = subprocess.Popen(self.cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        outs, errs = proc.communicate(input=bytes(input_text, encoding='utf-8'))
        output = outs.decode('utf-8')
        error_output = errs.decode('utf-8')
        returncode = proc.returncode

        if self.raise_exception and returncode != 0:
            raise Exception(f"External tool exited with the non-zero code {returncode}: {error_output.strip()}")

        if self.return_debug_data:
            return output, error_output, returncode

        return output


@contextmanager
def open_xz_or_gzip_or_plain(file_path, mode='rt'):
    f = None
    try:
        if file_path[-3:] == ".gz":
            f = gzip.open(file_path, mode)
        elif file_path[-3:] == ".xz":
            f = lzma.open(file_path, mode)
        else:
            f = open(file_path, mode)
        yield f

    except Exception:
        raise Exception("Error occurred while loading a file!")

    finally:
        if f:
            f.close()


@contextmanager
def dummy_open():
    yield None


def build_mappings(file_path_from, file_path_to, column=None, dem='\t'):
    mapping = {}

    def next_or_next_in_column(handler):
        if not column:
            return next(handler, None)

        text = next(handler, None)
        if text:
            return text.split(dem)[column]

        return text

    with open_xz_or_gzip_or_plain(file_path_from) as f_from, open_xz_or_gzip_or_plain(file_path_to) as f_to:
        line_from = next_or_next_in_column(f_from)
        line_to = next_or_next_in_column(f_to)

        while line_from and line_to:
            line_from = line_from.strip()
            mapping[line_from] = line_to.strip()

            line_from = next_or_next_in_column(f_from)
            line_to = next_or_next_in_column(f_to)

    return mapping


def check_lengths(file_path_from, file_path_to, throw=True):
    f1_lines = 0
    f2_lines = 0
    with open_xz_or_gzip_or_plain(file_path_from) as f:
        for _ in f:
            f1_lines += 1

    with open_xz_or_gzip_or_plain(file_path_to) as f:
        for _ in f:
            f2_lines += 1

    if throw and f1_lines != f2_lines:
        raise Exception("Files must have the same number of lines!\
                            {0}: {1}, {2}: {3}".format(file_path_from, f1_lines, file_path_to, f2_lines))

    return f1_lines == f2_lines


def check_connection(url):
    connection_error = False
    connection = None

    for check in range(2):
        try:
            connection = requests.get(url, timeout=15)
        except requests.exceptions.ConnectTimeout:
            if check:
                connection_error = True
            else:
                url = f"https{url[4:]}"
        except BaseException as e:
            if check:
                connection_error = True
                sys.stderr.write(f"WARNING: error connecting to {url}\n")
                sys.stderr.write(str(e) + "\n")

    return connection_error, url


duration_suffix = {"s": 1, "m": 60, "h": 3600, "d": 86400, "w": 604800}
# expected duration: integer + one-letter suffix
# allowed suffixes: s (seconds), m (minutes), h (hours), d (days), w (weeks)
def duration_to_seconds(value):
    suffix = value[-1]
    seconds = int(value[:-1]) * duration_suffix[suffix]
    return seconds


def return_dict_value_if_key(d, k, else_value, pos_value=None, only_check_key=False, apply_function=None):
    condition = k in d if only_check_key else k in d and d[k]

    if pos_value:
        result = pos_value if condition else else_value
    else:
        result = d[k] if condition else else_value

    if apply_function:
        result = apply_function(result)

    return result


def print_alternatively_lines(input_file="-", blocks=2):
    input_fd = sys.stdin if input_file == "-" else open(input_file)
    lines = []

    for line in input_fd:
        lines.append(line.strip())

    offset = len(lines) // blocks

    if len(lines) % blocks != 0:
        raise Exception(f"Provided lines mod blocks did not pass: {len(lines)} mod {blocks} != 0")

    for idx in range(len(lines)):
        if idx >= offset:
            break

        for i in range(blocks):
            print(lines[idx + offset * i])

    if input_file == "-":
        input_fd.close()


def get_all_idxs_from_list(l, element):
    idxs = []
    find_idx = 0

    while find_idx < len(l):
        try:
            idxs.append(l.index(element, find_idx))

            find_idx = idxs[-1] + 1
        except ValueError:
            find_idx = len(l)

    return idxs


def get_snakemake_execution_mark(tmp_path):
    mark_path = f"{tmp_path}/mark_first_execution"

    return mark_path


def is_first_snakemake_execution(tmp_path, create_mark=False):
    mark_path = get_snakemake_execution_mark(tmp_path)

    if path_exists(mark_path):
        return False

    # Create mark?
    if create_mark:
        with open(mark_path, "w"):
            pass

    return True


def path_exists(path, expand=True, f=os.path.isfile):
    _path = path

    if expand:
        _path = os.path.expanduser(path)

    return f(_path)
