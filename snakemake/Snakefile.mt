from os.path import join

#Output dirs
evaluation="{dir}/evaluation".format(dir=config["nmt-dir"])
print(evaluation)
corpus="{dir}/processed_corpus".format(dir=config["nmt-dir"])
modelDir="{dir}/model".format(dir=config["nmt-dir"])

marian=config["marian-dir"]
moses=config["moses-dir"]
subword_nmt=config["subword-nmt-dir"]
vocabSize=config["nmt-vocabSize"]
detokenizer=config["LANG2-detokenizer"]

#NMT commands
trainCmd = "{0}/build/marian -d {1}".format(marian, config["gpu-id"]) \
          + " --mini-batch-fit -w 2000 --optimizer-delay 2 --mini-batch 1000 --maxi-batch 1000" \
          + " --valid-log {dir}/valid.log".format(dir=modelDir) \
          + " --after-epochs 10 " \
          + " --log {dir}/train.log".format(dir=modelDir)

translateCmd = "{0}/build/marian-decoder -d {1}".format(marian, config["gpu-id"])

#Tokenization
tokenizer_l1= config["LANG1-tokenizer"]
tokenizer_l2= config["LANG2-tokenizer"]

#Input data prefixes
trainPath=config["nmt-train-prefix"]
devPath=config["nmt-dev-prefix"]
testPath=config["nmt-test-prefix"]

##########################################################################################################

if 'trainPath' in locals():
    #print("trainPath is defined")
    concatTrainPathFlattened = []
else:
    #print("trainPath is NOT defined")
    assert(concatTrainPath != None)

    numCorpora = len(concatTrainPath)
    #print("numCorpora", numCorpora)

    concatName = ""
    concatTrainPathFlattened = []

    for key, value in concatTrainPath.items():
        #print(key, value)
        if concatName != "":
            concatName += "+"
        concatName += key

        assert(len(value) == 2)
        sourceFile = value[0]
        targetFile = value[1]
        concatTrainPathFlattened.append(sourceFile)
        concatTrainPathFlattened.append(targetFile)

    trainPath = ["corpus/concat/{0}.{1}".format(concatName, {LANG1}),
                 "corpus/concat/{0}.{1}".format(concatName, {LANG2})
                ]

    #print("concatName", concatName)
    print("concatTrainPathFlattened", concatTrainPathFlattened)
    print("trainPath", trainPath)


############################################# EVALUATION #############################################################

def allTestNames(dataset):
    names = []
    for f in dataset:
        names.append(os.path.basename(f))
    return names

rule report:
    input:
        expand("{dir}/{name}.bleu", dir=evaluation, name=allTestNames(testPath))
    output:
        "{dir}/report".format(dir=evaluation)
    run:
        with open(output[0], "wt") as outHandle:
            for file in input:
                with open(file, "rt") as inHandle:
                    str = inHandle.read()
                    outHandle.write(os.path.basename(file).replace(".bleu",""))
                    outHandle.write("\t")
                    outHandle.write(str)

rule multibleu:
    input:
        trans="{dir}".format(dir=evaluation)+"/{name}.output.detokenized"
        ,
        ref="{dir}".format(dir=corpus)+"/test/{name}."+"{lang}".format(lang=LANG2)
    output:
        "{dir}".format(dir=evaluation)+"/{name}.bleu"
    shell:
        "cat {input.trans} | {moses}/scripts/generic/multi-bleu.perl {input.ref} > {output}"

##########################################  RUNNING MT ENGINE ##########################################

rule translate_test:
    input:
        model=directory("{dir}/marian".format(dir=modelDir))
        ,
        test="{pref}/test/".format(pref=corpus)+"{name}.bpe."+"{lang}".format(lang=LANG1)
    output:
        "{evaluation}".format(evaluation=evaluation)+"/{name}.output"
    shell:
        "cat {input.test} | {translateCmd} -c {input.model}/model.npz.decoder.yml > {output}"

rule train_nmt:
    input:
        vocab="{dir}/vocab.yml".format(dir=modelDir)
        ,
        train=["{pref}/train.bpe.{lang}".format(pref=corpus,lang=LANG1),
               "{pref}/train.bpe.{lang}".format(pref=corpus,lang=LANG2)]
        ,
        valid=["{pref}/dev.bpe.{lang}".format(pref=corpus,lang=LANG1),
               "{pref}/dev.bpe.{lang}".format(pref=corpus,lang=LANG2)]

    output:
        directory("{dir}".format(dir=modelDir)+"/marian")
    shell:
        "mkdir -p {output};"
        "{trainCmd} -t {input.train} --valid-sets {input.valid} --vocabs {input.vocab} {input.vocab} --early-stopping 10 -m {output}/model.npz"

################################################## MARIAN VOCAB ################################################################

rule make_vocab_yml:
    input:
        "{pref}".format(pref=corpus)+"/train.bpe."+"{lang}".format(lang=LANG1)
        ,
        "{pref}".format(pref=corpus)+"/train.bpe."+"{lang}".format(lang=LANG2)
    output:
        '{pref}'.format(pref=modelDir)+'/vocab.yml'
    shell:
        "cat {input} | {marian}/build/marian-vocab --max-size {vocabSize} > {output}"

####################################################### PREPROCESSING ###########################################################

rule apply_truecaser:
    input:
        file='{name}.tok.{lang}'
        ,
        model="{dir}/truecaser/".format(dir=modelDir)+"truecase-model.{lang}"
    output:
        '{name}.tc.{lang}'
    shell:
        "cat {input.file} | {moses}/scripts/recaser/truecase.perl -model {input.model} > {output}"

rule learn_truecaser:
    input:
        "{corpus}".format(corpus=corpus)+"/train.clean.{lang}"
    output:
        "{dir}/truecaser/".format(dir=modelDir)+"truecase-model.{lang}"
    shell:
        "mkdir -p {modelDir}/truecaser;"
        "{moses}/scripts/recaser/train-truecaser.perl -corpus {input} -model {output}"

rule clean:
    input:
        "{pref}.tok."+"{lang1}".format(lang1=LANG1)
        ,
        "{pref}.tok."+"{lang2}".format(lang2=LANG2)
    output:
        "{pref}.clean."+"{lang1}".format(lang1=LANG1)
        ,
        "{pref}.clean."+"{lang2}".format(lang2=LANG2)
    shell:
        "{moses}/scripts/training/clean-corpus-n.perl {wildcards.pref}.tok {LANG1} {LANG2} {wildcards.pref}.clean 1 80 {wildcards.pref}.lines-retained"

rule tokenize_file_l1:
    input: 
        "{pref}."+"{lang}".format(lang=LANG1)
    output:
        "{pref}.tok."+"{lang}".format(lang=LANG1)
    shell:
        "cat {input} | {tokenizer_l1} > {output}"

rule tokenize_file_l2:
    input:
        "{pref}."+"{lang}".format(lang=LANG2)
    output:
        "{pref}.tok."+"{lang}".format(lang=LANG2)
    shell:
        "cat {input} | {tokenizer_l2} > {output}"


####################################################### POSTPROCESSING ###########################################################

rule detok:
    input:
        "{pref}.output.detruecased"
    output:
        "{pref}.output.detokenized"
    shell:
        "cat {input} | {detokenizer} > {output}"

rule detruecase:
    input:
        "{pref}.output.debpe"
    output:
        "{pref}.output.detruecased"
    shell:
        "cat {input} | {moses}/scripts/recaser/detruecase.perl > {output}"

rule debpe:
    input:
        "{pref}.output"
    output:
        "{pref}.output.debpe"
    shell:
        "cat {input} | sed -r 's/(@@ )|(@@ ?$)//g' > {output}"

############################################## BPE ##############################################

rule apply_bpe:
    input:
        file="{pref}.tc.{lang}"
        ,
        vocab="{dir}/".format(dir=modelDir)+"vocab.{lang1}{lang2}".format(lang1=LANG1, lang2=LANG2)
    output:
        "{pref}.bpe.{lang}"
    shell:
        "{subword_nmt}/subword_nmt/apply_bpe.py -c {input.vocab} < {input.file} > {output}"

rule learn_bpe:
    input:
        "{pref}".format(pref=corpus)+"/train.clean."+"{lang}".format(lang=LANG1)
        ,
        "{pref}".format(pref=corpus)+"/train.clean."+"{lang}".format(lang=LANG2)
    output:
        '{dir}'.format(dir=modelDir)+'/vocab.'+'{lang1}{lang2}'.format(lang1=LANG1, lang2=LANG2)
    shell:
        "cat {input} | {subword_nmt}/subword_nmt/learn_bpe.py -s {vocabSize}  > {output}"

###################################### PREPARE DATA ##############################################

rule prepare_traindata:
    input:
         l1=expand("{dataset}.{lang}", dataset=trainPath, lang=LANG1)
         ,
         l2=expand("{dataset}.{lang}", dataset=trainPath, lang=LANG2)
    output:
         l1="{corpus}/train.{lang}".format(corpus=corpus, lang=LANG1)
         ,
         l2="{corpus}/train.{lang}".format(corpus=corpus, lang=LANG2)
    shell:
         "mkdir -p {corpus}; cat {input.l1} > {output.l1} && cat {input.l1} > {output.l2}"

rule prepare_devdata:
    input: 
         l1=expand("{dataset}.{lang}", dataset=devPath, lang=LANG1)
         ,
         l2=expand("{dataset}.{lang}", dataset=devPath, lang=LANG2)
    output: 
         l1="{corpus}/dev.{lang}".format(corpus=corpus, lang=LANG1)
         ,
         l2="{corpus}/dev.{lang}".format(corpus=corpus, lang=LANG2)
    shell:
         "mkdir -p {corpus}; cat {input.l1} > {output.l1} && cat {input.l2} > {output.l2}"

rule prepare_test:
    input:
         l1=expand("{dataset}.{lang}", dataset=testPath, lang=LANG1)
         ,
         l2=expand("{dataset}.{lang}", dataset=testPath, lang=LANG2)
    output:
         expand("{corpus}/test/{name}.{lang}", corpus=corpus, name=allTestNames(testPath), lang=LANG1)
         ,
         expand("{corpus}/test/{name}.{lang}", corpus=corpus, name=allTestNames(testPath), lang=LANG2)
    shell:
         "mkdir -p {corpus}/test; cp -r {input.l1} {input.l2} {corpus}/test"

rule decompress:
    input:
         "{pref}.{lang}.gz"
    output:
         temp("{pref}.{lang}")
    wildcard_constraints:
         lang="({l1}|{l2})".format(l1=LANG1, l2=LANG2)
    shell:
         "zcat {input} > {output}"
