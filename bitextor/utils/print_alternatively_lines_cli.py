#!/usr/bin/env python3

import sys

from common import print_alternatively_lines

if __name__ == "__main__":
    blocks = 2

    if len(sys.argv) > 1:
        blocks = int(sys.argv[1])

    print_alternatively_lines(input_file="-", blocks=blocks)
