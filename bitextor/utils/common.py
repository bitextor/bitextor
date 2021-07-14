try:
    import lzma
except ImportError:
    from backports import lzma
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

import gzip
from contextlib import contextmanager

import subprocess
import psutil
import sys
import os


class ExternalTextProcessor(object):

    def __init__(self, cmd):
        self.cmd = cmd

    def process(self, input_text):
        proc = subprocess.Popen(self.cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        outs, errs = proc.communicate(input=bytes(input_text, encoding='utf-8'))

        return outs.decode('utf-8'), errs.decode('utf-8'), proc.returncode


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


def get_all_ppids(pid, append_pid=False):
    result = []

    if append_pid:
        result.append(pid)

    if not isinstance(pid, int):
        raise Exception("PID must be an integer")

    p = pid

    while p != 1:
        p = psutil.Process(p).ppid()
        result.append(p)

    return result


def snake_no_more_race_get_pgid():
    command = f"ps axo pid,pgid,comm | grep -E \"snakemake$|python3[.]8$\""
    pgid = subprocess.getoutput(
        f"{command} | grep \\ {os.getpgid(os.getpid())}\\ | awk '{{print $1}}' | grep {os.getpid()}")

    all_ppids = get_all_ppids(os.getpid())
    all_pgids = list(map(lambda pid: os.getpgid(pid), all_ppids))

    if len(pgid) == 0:
        for pid in all_ppids:
            pgid = subprocess.getoutput(f"{command} | grep \\ {os.getpgid(pid)}\\ | awk '{{print $1}}' | grep {pid}")

            if len(pgid) != 0:
                break
    elif all_ppids[0] != all_pgids[0]:
        idx = 0
        while idx < len(all_ppids):
            if all_ppids[idx] == all_pgids[idx]:
                pgid = str(all_pgids[0])
                break
            idx += 1

        if idx == len(all_ppids):
            pgid = subprocess.getoutput(
                f"{command} | grep \\ {os.getpgid(os.getpid())}\\ | awk '{{print $1}}' | grep {os.getpid()}")
            sys.stderr.write(
                f"WARNIGN: could not get the process group leader of {os.getpid()}. The PID gathering might be incorrect")

    return pgid


def snake_no_more_race_get(file_path):
    value = None

    if os.path.isfile(file_path):
        f = open(file_path)
        file_pgid = f.readline().strip()
        pgid = snake_no_more_race_get_pgid()

        if len(pgid) == 0:
            sys.stderr.write("WARNING: could not get the PGID. Using 4321 as default value\n")
            pgid = "4321"

        if file_pgid == pgid:
            value = f.readline().strip()
        else:
            os.unlink(file_path)

        f.close()

    return value


def snake_no_more_race_set(file_path, value):
    if not os.path.isfile(file_path):
        f = open(file_path, "w")
        pgid = snake_no_more_race_get_pgid()

        if len(pgid) == 0:
            sys.stderr.write("WARNING: could not get the PGID. Using 1234 as default value\n")
            pgid = "1234"

        f.write(f"{pgid}\n")
        f.write(f"{value}\n")

        f.close()

        return True
    return False
