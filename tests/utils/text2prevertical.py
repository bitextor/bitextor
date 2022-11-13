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

import math
import random
import base64
import logging
import argparse
from datetime import datetime

from common import open_xz_or_gzip_or_plain

def roulette_wheel_selection(values, likelihoods):
    sorted_values = sorted(zip(values, likelihoods), key=lambda x: x[1])
    accumulated_likelihood = 0.0

    # Set accumulated likelihood (last element likelihood = 1.0)
    for idx, (value, likelihood) in enumerate(sorted_values):
        accumulated_likelihood += likelihood

        sorted_values[idx] = (value, accumulated_likelihood)

    r = random.random()

    for value, likelihood in sorted_values:
        if r < likelihood:
            return value

    return sorted_values[-1][0]

def text2prevertical(text_files, url_files, langs, langs_likelihood, boilerplate_likelihood, min_lang_diff_likelihood,
                     random_date=False, seed=-1):
    random.seed(seed if seed >= 0 else None)

    for text_file, url_file in zip(text_files, url_files):
        with open_xz_or_gzip_or_plain(text_file) as text_fd, open_xz_or_gzip_or_plain(url_file) as url_fd:
            for doc_idx, (doc, url) in enumerate(zip(text_fd, url_fd), 1):
                doc = doc.strip()
                url = url.strip().replace('\t', ' ')
                content = ""
                prevertical_content = ""

                if random_date:
                    _year = random.randint(1970, 2021)
                    _month = random.randint(1, 12)
                    _day = random.randint(1, 25)
                    _hour = random.randint(0, 23)
                    _mins = random.randint(0, 59)

                    current_date = f"{_year}/{_month}/{_day} {_hour}:{_mins}"
                else:
                    current_date = datetime.now().strftime("%Y/%m/%d %H:%M")

                try:
                    content = base64.b64decode(doc.strip()).decode("utf-8")
                except UnicodeDecodeError:
                    logging.warning("unicode decoding error while processing doc #%d", doc_idx)

                    continue

                title = "this is the title"
                # TODO is it the length format correct?
                length = f"{len(content)}" if len(content) < 1024 else f"{len(content) // 1024}k"
                lang = roulette_wheel_selection(langs, langs_likelihood)
                lang_diff = f"{min(random.uniform(min_lang_diff_likelihood, 1.00005), 1.0):.2f}"
                ip = f"{random.randint(0, 256)}.{random.randint(0, 256)}.{random.randint(0, 256)}.{random.randint(0, 256)}"

                # Greedy document header
                prevertical_content += f"<doc id=\"{doc_idx}\" title=\"{title}\"" \
                                       f" length=\"{length}\" crawl_date=\"{current_date}\"" \
                                       f" lang=\"{lang}\" lang_diff=\"{lang_diff}\"" \
                                       f" ip=\"{ip}\" url=\"{url}\" file_type=\"html\"" \
                                       f" enc_meta=\"utf-8\" enc_chared=\"utf-8\">\n"

                paragraphs = content.strip().replace('\t', ' ').split('\n')
                printed_paragraphs = 0

                for paragraph in paragraphs:
                    paragraph = paragraph.strip()

                    if paragraph == '':
                        continue

                    # Escape HTML entities which might be harmful for XML
                    #  don't escape '&' since we might escape twice previous escaped HTML entities
                    paragraph = paragraph.replace('<', '&lt;') \
                                         .replace('>', '&gt;') \
                                         .replace('\'', '&apos;') \
                                         .replace('"', '&quot;')

                    lang_diff = f"{min(random.uniform(min_lang_diff_likelihood, 1.00005), 1.0):.2f}"
                    boilerplate = "good" if random.random() > boilerplate_likelihood else "bad"

                    # Greedy paragraph header
                    prevertical_paragraph = f"<p class=\"{boilerplate}\" cfclass=\"{boilerplate}\" langdiff=\"{lang_diff}\">\n"

                    prevertical_paragraph += paragraph
                    prevertical_paragraph += "\n</p>\n"

                    prevertical_content += prevertical_paragraph

                    printed_paragraphs += 1

                prevertical_content += "</doc>"

                if printed_paragraphs != 0:
                    print(prevertical_content)

def parse_args():
    parser = argparse.ArgumentParser(description="It joins the sentences of the content from BASE64 documents and returns a new BASE64-encoded document",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('--text-files', nargs='+', required=True,
                        help="Text files (base64-encoded documents)")
    parser.add_argument('--url-files', nargs='+', required=True,
                        help="URL files")

    parser.add_argument('--min-lang-diff-likelihood', default=0.0, type=float,
                        help="Min. likelihood of the lang-diff attribute")
    # Documents
    parser.add_argument('--document-langs', nargs='+', default=['English', 'French'],
                        help="Language of the documents")
    parser.add_argument('--document-langs-likelihood', nargs='*', type=float,
                        help="Likelihood of the languages of the documents. The provided values has to add up to 1")
    parser.add_argument('--random-date', action="store_true",
                        help="Set random date instead of current date (useful for deterministic results if --seed is provided)")
    # Sentences
    parser.add_argument('--sentence-boilerplate-likelihood', default=0.2, type=float,
                        help="Likelihood of a sentence to be set as boilerplate")

    parser.add_argument('--seed', type=int, default=-1,
                        help="Seed for reproducibility. Has to be >= 0")
    parser.add_argument('-v', '--verbose', action="store_true",
                        help="Verbose logging")

    args = parser.parse_args()

    return args

if __name__ == '__main__':
    args = parse_args()

    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)

    if len(args.text_files) != len(args.url_files):
        raise Exception(f"Different number of arguments provided to --text-files and --url-files")
    if not args.document_langs_likelihood:
        args.document_langs_likelihood = [1.0 / len(args.document_langs)] * len(args.document_langs)
    if len(args.document_langs) != len(args.document_langs_likelihood):
        raise Exception(f"Different number of arguments provided to --document-langs and --document-langs-likelihood")
    if not math.isclose(sum(args.document_langs_likelihood), 1.0):
        raise Exception(f"The provided values of --document-langs-likelihood has to add up to 1")

    args.sentence_boilerplate_likelihood = min(args.sentence_boilerplate_likelihood, 1.0)
    args.min_lang_diff_likelihood = min(args.min_lang_diff_likelihood, 1.0)

    text2prevertical(args.text_files, args.url_files, args.document_langs, args.document_langs_likelihood,
                     args.sentence_boilerplate_likelihood, args.min_lang_diff_likelihood, random_date=args.random_date,
                     seed=args.seed)
