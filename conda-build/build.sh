
echo -e "\e[4m\e[1mBuilding $PKG_NAME...\e[0m"

pushd "bitextor" > /dev/null

# Go dependencies
echo -e " - \e[4mInstalling Go dependencies...\e[0m"
mkdir "$SRC_DIR/../gopath"
export GOPATH="$SRC_DIR/../gopath"
go get github.com/paracrawl/giashard/...

# Pip dependencies
echo -e " - \e[4mInstalling python dependencies...\e[0m"
export PIP_NO_INDEX="False" # We are downloading requisites from PyPi
export PIP_NO_DEPENDENCIES="False" # We need the dependencies from our defined dependencies
export PIP_IGNORE_INSTALLED="False" # We need to take into account the dependencies

# Hunspell (conda package fix)
if [[ ! -f $PREFIX/lib/libhunspell.so ]]; then
  ln -s $PREFIX/lib/libhunspell{-1.7,}.so
fi
if [[ ! -f $PREFIX/lib/libhunspell.a ]]; then
  ln -s $PREFIX/lib/libhunspell{-1.7,}.a
fi
$PYTHON -m pip install pip Cython setuptools wheel -U
$PYTHON -m pip install .[all]
### FastSpell (bicleaner-hardrules)
INCLUDE_PATH="$PREFIX/include" $PYTHON -m pip install hunspell
### Bicleaner, Bicleaner AI and KenLM
$PYTHON -m pip install ./third_party/bicleaner
$PYTHON -m pip install ./third_party/bicleaner-ai
$PYTHON -m pip install ./third_party/kenlm --install-option="--max_order 7"
### Bifixer
$PYTHON -m pip install ./third_party/bifixer
### Biroamer and model
$PYTHON -m pip install ./third_party/biroamer && \
$PYTHON -c "from flair.models import SequenceTagger; SequenceTagger.load('flair/ner-english-fast')"
### Neural Document Aligner and Vecalign
$PYTHON -m pip install ./third_party/neural-document-aligner
$PYTHON -m pip install ./third_party/vecalign
## CLD3
$PYTHON -m pip install pycld3

echo -e " - \e[4mMake...\e[0m"
mkdir -p build && cd build
CPATH="$PREFIX/include:$CPATH" cmake "-DCMAKE_INSTALL_PREFIX=$PREFIX" ..
CPATH="$PREFIX/include:$CPATH" LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH" make -j install

popd > /dev/null

# Heritrix 3 - handled in meta.yaml

echo -e " - \e[4mPost-build actions...\e[0m"
# Clean
find . -type d -or -type l | grep /__pycache__$ | xargs -I{} rm -rf {}
find . -type f -or -type l | grep [.]o$ | xargs -I{} rm -rf {}
find . -type d -or -type l | grep /CMakeFiles$ | xargs -I{} rm -rf {}

# Copy python scripts, necessary data and 3rd packages
mkdir -p "$PREFIX/bitextor"
mkdir -p "$PREFIX/bitextor/third_party"

# Bitextor files
cp -r "$SRC_DIR/bitextor/bitextor" "$PREFIX/bitextor" # Scripts and data
cp -r "$SRC_DIR/bitextor/tests" "$PREFIX/bitextor" # Tests
cp -r "$SRC_DIR/bitextor/requirements" "$PREFIX/bitextor" # Requirements
cp "$SRC_DIR/bitextor/requirements.txt" "$PREFIX/bitextor" # Requirements
## Biroamer
cp -r "$SRC_DIR/bitextor/third_party/biroamer" "$PREFIX/bitextor/third_party" # Fast Align binaries are not correctly placed

# 3rd party
cp -r "$SRC_DIR/../gopath" "$PREFIX"
cp -r "$SRC_DIR/heritrix-3.4.0-SNAPSHOT" "$PREFIX"

# Soft links (necessary to be relative, not absolute)
pushd "$PREFIX/bin" > /dev/null

ln -s ../gopath/bin/giashard ./giashard || \
  >&2 echo -e " - \e[31m\e[4mpost-build-act: could not create link 'giashard'...\e[0m"
ln -s "$PREFIX/heritrix-3.4.0-SNAPSHOT" "$PREFIX/heritrix3" || \
  >&2 echo -e " - \e[31m\e[4mpost-build-act: could not create link 'heritrix3' (directory)...\e[0m"

# Restore initial working directory (trying to prevent errors)
popd > /dev/null

echo -e "\e[4m\e[1mBuild done!\e[0m"
