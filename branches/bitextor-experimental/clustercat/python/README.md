# Python ClusterCat


## Installation
First follow the [installation instructions](../README.md) in the above directory.
After that, you normally don't need to install anything here.  You can load the module `clustercat` using either Python 2 or 3.

    cd python
    python3
    >>> import clustercat as cc
    >>> clustering = cc.cluster(text=['this is a test', 'that is only a test', 'bye'], min_count=1)
    >>> print(clustering)

If you get an error message saying that it is unable to access clustercat binary, follow all the instructions in the error message.
You'll need more text input than the toy example above to produce useful clusters.

To import this module from a different directory, you can add the module's directory to `$PYTHONPATH`:

    cd python
	echo "export PYTHONPATH=\$PYTHONPATH:`pwd`" >> ~/.bashrc
	source ~/.bashrc

## Python ClusterCat Functions
### `cluster(text=None, in_file=None, ...)`
Produce a clustering, given a textual input.  There is one required argument (the training input text), and many optional arguments.  The one required argument is **either** `text` **or** `in_file`.  The argument `text` is a list of Python strings.  The argument `in_file` is a path to a text file, consisting of preprocessed (eg. tokenized) one-sentence-per-line text.  The use of `text` is probably not a good idea for large corpora.

```Python
cc.cluster(text=['this is a test', 'that is only a test', 'bye'], min_count=1)
cc.cluster(in_file='/tmp/corpus.tok.txt', min_count=3)
```

The other optional arguments are described by running the compiled clustercat binary with the `--help` argument, except that the leading `--` from the shell argument is removed, and `-` is replaced with `_`.  So for example, instead of `--tune-cycles 15`, the Python function argument would be `tune_cycles=15` .

Returns a dictionary of the form `{ word : cluster_id }` .


### `save(mapping, out, format='tsv')`
Save a clustering (dictionary) to file.  By default the output file is a tab-separated listing of words and their cluster ID.

```Python
cc.save(clustering, 'clusters.tsv')
```


### `load(in_file, format="tsv")`
Load a clustering from a file.  By default the input file is a tab-separated listing of words and their cluster ID.
Returns a dictionary of the clustering.

```Python
clustering = cc.load('clusters.tsv')
```


### `tag_string(mapping, text, unk="<unk>")`
Tag a string with the corresponding cluster ID's.  If a word is not found in the clustering, use `unk`.
Returns a string.

```Python
tagged_sent = cc.tag_string(clustering, "this is a test")
```

### `tag_stdin(mapping, unk="<unk>")`
This calls `tag_string()` for each line in `stdin`, and prints the result to `stdout`.
