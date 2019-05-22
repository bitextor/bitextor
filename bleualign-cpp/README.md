# Bleualign-cpp
C++ sentence alignment tool based on [Bleualign](https://github.com/rsennrich/Bleualign).
Bleualign-cpp is expected to be used together with [document-aligner](https://github.com/bitextor/bitextor/tree/master/document-aligner).

### Requirements
- GCC, C++11 compiler
- [Boost](https://www.boost.org/) 1.58.0 or later
- [CMake](https://cmake.org/download/) 3.7.2 or later
- [GTest](https://github.com/google/googletest) (for tests)

### Compile with CMake

```bash
mkdir build
cd build
cmake .. -DBUILD_TEST=on -DCMAKE_BUILD_TYPE=Release
make -j 4
tests/test_all
```


### Usage

Bleualign-cpp takes two files containing texts in two different languages and aligns them to produce a parallel text. To this end, it also needs a translation of one of these text files. This translation must correspond line-by-line with the original text while keeping the IDs unchanged. The input format for all files is `document-id \t sentence` per line.

Bleualign-cpp outputs one *aligned.\<suffix\>.gz* file per document in a separate folder (--output_dir parameter), and the suffix-to-ID alignments are stored in *align\.info* file.

```bash
./bleualign-cpp
```

##### Required Parameters
* **--text1** - An input file in language1
* **--text2** - An input file in language2
* **--text1translated** - Translated content of the first input file (language1->language2)
* **--matches** - A document containing IDs of matched documents with scores.
Format: `score \t ID1 \t ID2`
* **--output_dir** - A path to the output directory. The *aligned.\*.gz* files will be outputted there.

##### Optional Parameters
* **--matches_threshold** - Documents that score lower than the threshold will be skipped. (Default: 0.0)
* **--bleu_threshold** - Sentence-level BLEU score threshold (Default: 0.0)
