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

# Non-generic files (used in concrete places and moments)

import tldextract
from itertools import product

def create_domain_key_2_host_map(hosts):
    key2hosts = {}
    for host in hosts:
        # don't merge blog sites
        if host.find(".blogspot.") >= 0 or host.find(".wordpress.") >= 0:
            key = host
        else:
            key = tldextract.extract(host).domain

        if key not in key2hosts:
            key2hosts[key] = []
        key2hosts[key].append(host)
    return key2hosts


def parent_folder_2_warcs(warcs):
    f2w = {}
    for warc in warcs:
        folder = warc.split('/')[-2]
        if folder not in f2w:
            f2w[folder] = []
        f2w[folder].append(warc)
    return f2w


def get_lang_or_default(scripts_dict, language):
    cmd = ""
    if language in scripts_dict:
        cmd = scripts_dict[language]
    elif "default" in scripts_dict:
        cmd = scripts_dict["default"]
    return cmd


def get_customnbp(nbp_dict, language):
    nbp = ""
    if language in nbp_dict:
        nbp = nbp_dict[language]
    return nbp

def get_mt_docalign_inputs(src_batches, trg_batches):
    # product( [[shard, batch], [shard, batch], ...], [[shard, batch], [shard, batch], ...] )
    iterator = product( [batch.split('/')[-2:] for batch in src_batches], [batch.split('/')[-2:] for batch in trg_batches] )
    # each item -> (shard, (src_batch, trg_batch))
    return [(src_shard, (src_batch, trg_batch)) for ((src_shard, src_batch), (trg_shard, trg_batch)) in iterator if src_shard == trg_shard]
