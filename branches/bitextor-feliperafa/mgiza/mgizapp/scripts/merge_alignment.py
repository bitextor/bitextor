#!/usr/bin/env python
# Author : Qin Gao
# Date   : Dec 31, 2007
#
# This file is part of mgiza++.  Its use is licensed under the GNU General
# Public License version 2 or, at your option, any later version.

"""Combine multiple alignment files into a single one.

The files are prodcuced by MGIZA, which has sentence IDs, and every file is
ordered inside.
"""

from __future__ import unicode_literals
import sys
import re
import codecs
import io
import os


def normalize_path(path):
    """Normalize a filesystem path.

    Convert Windows/Unix path separators to native ones, support "~" for
    home directory portably, and convert the path to an absolute.
    """
    path = path.replace('\\', os.sep).replace('/', os.sep)
    path = os.path.expanduser(path)
    path = os.path.abspath(path)
    return path


ID_PATTERN = re.compile("\\((\\d+)\\)")


def extract_id(line):
    """Extract a sentence ID from `line`."""
    match = ID_PATTERN.search(line)
    return int(match.group(1))


def main():
    """Main body."""
    if sys.version_info < (3, 0, 0):
        sys.stdin = codecs.getreader('UTF-8')(sys.stdin)
        sys.stdout = codecs.getwriter('UTF-8')(sys.stdout)
        sys.stderr = codecs.getwriter('UTF-8')(sys.stderr)
    else:
        sys.stdout = open(sys.stdout.fileno(), mode='w', encoding='utf8', buffering=1)

    if len(sys.argv) < 2:
        sys.stderr.write("Provide me the file names (at least 2)\n")
        sys.exit()

    sent_id = 0

    files = []
    ids = []

    sents = []
    done = []

    for i in range(1, len(sys.argv)):
        fname = normalize_path(sys.argv[i])
        files.append(io.open(fname, "r", encoding="UTF-8"))
        ids.append(0)
        sents.append("")
        done.append(False)

    i = 0
    while i < len(files):
        st1 = files[i].readline()
        st2 = files[i].readline()
        st3 = files[i].readline()
        if len(st1) == 0 or len(st2) == 0 or len(st3) == 0:
            done[i] = True
        else:
            ids[i] = extract_id(st1)
            sents[i] = (st1, st2, st3)
        i += 1

    cont = True
    while cont:
        sent_id += 1
        write_one = False
# Now try to read more sentences
        i = 0
        cont = False
        while i < len(files):
            if done[i]:
                i += 1
                continue
            cont = True
            if ids[i] == sent_id:
                sys.stdout.write(
                    "%s%s%s" % (sents[i][0], sents[i][1], sents[i][2]))
                write_one = True
                st1 = files[i].readline()
                st2 = files[i].readline()
                st3 = files[i].readline()
                if len(st1) == 0 or len(st2) == 0 or len(st3) == 0:
                    done[i] = True
                else:
                    ids[i] = extract_id(st1)
                    sents[i] = (st1, st2, st3)
                    cont = True
                break
            elif ids[i] < sent_id:
                sys.stderr.write("ERROR! DUPLICATED ENTRY %d\n" % ids[i])
                sys.exit()
            else:
                cont = True
            i += 1
        if (not write_one) and cont:
            sys.stderr.write("ERROR! MISSING ENTRy %d\n" % sent_id)
            sys.exit(1)
    sys.stderr.write(
        "Combined %d files, totally %d sents \n" % (len(files), sent_id - 1))


if __name__ == '__main__':
    main()
