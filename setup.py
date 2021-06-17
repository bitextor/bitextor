#!/usr/bin/env python

import setuptools
import os, shutil

def copytree(src, dst):
    os.makedirs(dst, exist_ok=True)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            copytree(s, d)
        else:
            shutil.copy(s, d)

if __name__ == "__main__":
    with open("docs/README.md", "r") as fh:
        long_description = fh.read()

    requirements_files=["requirements.txt", "requirements/requirements-boilerpipe.txt", "requirements/requirements-dict-aligner.txt", "requirements/requirements-pdfextract.txt", "requirements/requirements-w2p.txt"]
    requirements=[]
    wd = os.path.dirname(os.path.abspath(__file__))

    copytree("preprocess/moses", os.path.join(wd, "workflow/data/moses"))

    for req_file in requirements_files:
        with open(req_file) as rf:
            for line in rf:
                if not line.startswith("-r"):
                    requirements.append(line.strip())

    setuptools.setup(
        name="bitextor",
        version="8.0",
        install_requires=requirements,
        license="GNU General Public License v3.0",
        #author=,
        #author_email=,
        #maintainer=,
        #maintainer_email,
        description="Bitextor generates translation memories from multilingual websites",
        long_description=long_description,
        long_description_content_type="text/markdown",
        url="https://github.com/bitextor/bitextor",
        package_dir={'bitextor': 'workflow'},
        packages=["bitextor", "bitextor.utils", "bitextor.features"],
        #classifiers=[],
        #project_urls={},
        package_data={
            "bitextor": [
                "Snakefile",
                "bitextor-webdir2warc.sh",
                "rules/*",
                "utils/clean-corpus-n.perl",
                "data/*",
                "data/model/*",
                "data/moses/ems/support/*",
                "data/moses/tokenizer/*",
                "data/moses/share/nonbreaking_prefixes/*",
            ]
        },
        entry_points={
            "console_scripts": [
                "bitextor = bitextor.bitextor:main"
            ]
        }
        )
