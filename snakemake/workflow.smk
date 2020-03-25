include: "crawling.smk"
if False:
	include: "preprocessing.smk"

rule all:
	input: "/home/elsa/permanent/data/preprocess/greenpeace/w2p/en/plain_text.gz"
