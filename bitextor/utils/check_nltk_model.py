
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
        # E.g. corpus/reader/wordlist/perluniprops perluniprops
        logging.error("Syntax: <model_path> <path> [quiet]")

        sys.exit(1)

    model_path = sys.argv[1].strip()
    model = sys.argv[2].strip()
    quiet = False

    if len(sys.argv) > 3:
        if sys.argv[3].lower() == "true":
            quiet = True

    check(model_path, model, quiet=quiet)
