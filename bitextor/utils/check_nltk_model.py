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

import sys
import logging

import nltk

def check(model_path, model, download=True, quiet=False):
    try:
        nltk.data.find(model_path)
    except LookupError:
        logging.info("NLTK model not available: %s", model)

        if download:
            logging.info("Downloading model")

            nltk.download(model, quiet=quiet)


if __name__ == '__main__':
    if len(sys.argv) < 3:
        # E.g. misc/perluniprops perluniprops True
        logging.error("Syntax: <model_path> <path> [quiet]")

        sys.exit(1)

    model_path = sys.argv[1].strip()
    model = sys.argv[2].strip()
    quiet = False

    if len(sys.argv) > 3:
        if sys.argv[3].lower() == "true":
            quiet = True

    check(model_path, model, quiet=quiet)
