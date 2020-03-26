include: "crawling.smk"

# by manually manipulating the config it is possible to connect different workflows
if "warcs" not in config:
	config["warcs"] = []
config["warcs"].extend(rules.crawling_all.input)

include: "preprocessing.smk"

rule all:
	input: rules.preprocess_all.input
