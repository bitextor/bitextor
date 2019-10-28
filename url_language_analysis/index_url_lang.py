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

import glob
import lzma


d = {}
lang_pair_path_ls = glob.glob("/home/s0565741/workspace/experiment/paracrawl/v3/*-*/")

print("Found language pairs:")
print(lang_pair_path_ls)
for lang_pair_path in lang_pair_path_ls:
    lang_pair = lang_pair_path.replace("/home/s0565741/workspace/experiment/paracrawl/v3/", "")
    lang_1, lang_2 = lang_pair[:2], lang_pair[3:5]
    print("Processing " + lang_1 + "-" + lang_2)
    domain_ls = glob.glob(lang_pair_path + "transient/*")
    for domain in domain_ls:
        # print(domain_ls)
        path = domain + "/bleualign.bicleaner.xz"
        try:
            with lzma.open(path) as f:
                for line in f:
                    ls = line.split()
                    url_1, url_2 = ls[0], ls[1]
                    #print(line)
                    #print(ls)
                    #print(url_1, url_2)
                    for url, lang in [(url_1, lang_1), (url_2, lang_2)]:
                        if url in d:
                            if not lang in d[url]:
                                d[url].append(lang)
                        else:
                            d[url] = [lang]
        except:
            continue

print("Finished Reading")
f = open("out.txt", "w")
urls = d.keys()
for url in urls:
    f.write(url.decode("utf-8") + "\n")
    f.write("\t\t"+ ", ".join(d[url]) + "\n")
f.close



'''
IDEA:

build a dict = {
   url_1: [lang_1, lang_2, lang_3]
   url_2: [lang_1, lang_4]
   url_3: [lang_1]
   ......
}

loop through all language pairs (source, target):
    loop through all domain_folders:
        open bleualign.bicleaner.xz:
        loop through all lines:
            get url_source, url_target
            for lang in [source, target]:
                if dict[url_lang]:
                    if not lang not in dict[url_lang]:
                        dict[url_source].append(lang)
                else:
                    dict[url_lang] = [lang]
'''
