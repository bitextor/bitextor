# Document aligner

### Setup with a virtual environment (optional but recommended)

Since Python 3.5, a virtual environment can be created and activated by executing the following commands:
```bash
python3 -m venv venv
source venv/bin/activate
```
### Install

Next, in the your environment (virtual or not), the following packages need to be installed:
- [chared](http://corpus.tools/wiki/Chared)
```bash
wget http://corpus.tools/raw-attachment/wiki/Downloads/chared-1.2.2.tar.gz
tar xzvf chared-1.2.2.tar.gz chared-1.2.2 && cd chared-1.2.2
python3 setup.py install
#Ignore risen errors, if any. Python 3 compatible parts are enough for this project.
```
- [cld2](https://github.com/CLD2Owners/cld2)
```bash
git clone https://github.com/CLD2Owners/cld2.git && cd cld2
export CLD2_PATH=`pwd`
cd internal && ./compile_libs.sh
```
- [python-cld2](https://github.com/scrapinghub/python-cld2)
```bash
sudo apt-get install python3-dev python-dev #In case of Ubuntu/Debian based
git clone https://github.com/scrapinghub/python-cld2.git && cd python-cld2
export LIBRARY_PATH=${LIBRARY_PATH}:${CLD2_PATH}/internal/
python3 setup.py build && python setup_full.py build
python3 setup.py install && python setup_full.py install
echo "export LD_LIBRARY_PATH=\${LD_LIBRARY_PATH}:${CLD2_PATH}/internal/" >> ~/.bashrc
```

Re-source .bashrc and re-activate the environment:

```bash
. ~/.bashrc
source venv/bin/activate
```

Finally, the rest of the required python packages can be installed from the *requirements.txt* file:

```bash
pip3 install -r requirements.txt
```


### Document alignment

The document aligner takes a LETT file and produces *en-\*.matches* file containing a list of references to the aligned documents. Optionally, the content of the matched documents can be written in the standard output.

```bash
doc_align.sh -f <lett_file> -l <foreign_language> -t <translation_script> -w <working_directory> [-s <score_threshold>] [-b <batch_size>] [-d]
```

##### Required Parameters
* **-f <string>**: An input file in the LETT format.
* **-l <string>**: A language code of the foreign language occurring in the LETT file.
* **-t <string>**: A path to a bash script that takes text in the foreign language as its standard input and outputs an English translation to its standard output.
* **-w <string>**: A path to the working directory. The *en-\*.matches* file will be outputted there.

##### Optional Parameters
* **-s <float>**: Documents that score lower than the threshold will be skipped. Default: 0.1
* **-b <int>**: Batch size for the distance matrix computation. Use lower values if you encounter memory issues. Default: 1000
* **-d**: Writes the content of the matched documents to the standard output in the following format: `url_sl \t url_tl \t doc_sl_encoded_base64 \t doc_tl_encoded_base64`
