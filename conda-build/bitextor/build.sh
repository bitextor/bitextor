
echo -e "\e[4m\e[1mBuilding Bitextor...\e[0m"

cd "bitextor"

# Go dependencies
echo -e " - \e[4mInstalling Go dependencies...\e[0m"
mkdir "$PWD/../gopath"
go env
export GOPATH="$PWD/../gopath"
go get github.com/paracrawl/giashard/...

# Pip dependencies
echo -e " - \e[4mInstalling python3 dependencies...\e[0m"
export PIP_NO_INDEX="False" # We are downloading requisites from PyPi
export PIP_NO_DEPENDENCIES="False" # We need the dependencies from our defined dependencies
export PIP_IGNORE_INSTALLED="False" # We need to take into account the dependencies

pip3 install .
### Bicleaner and KenLM
pip3 install ./bicleaner
pip3 install ./bicleaner-ai
pip3 install ./kenlm --install-option="--max_order 7"
### Bifixer
pip3 install ./bifixer
### Biroamer and model
pip3 install ./biroamer && \
python3 -c "from flair.models import SequenceTagger; SequenceTagger.load('flair/ner-english-fast')"
### CLD3
pip3 install Cython
pip3 install pycld3
### Linguacrawl
pip3 install git+https://github.com/transducens/linguacrawl.git

echo -e " - \e[4mMake...\e[0m"
mkdir -p build && cd build
CPATH="$CPATH:$CONDA_PREFIX/include" cmake "-DCMAKE_INSTALL_PREFIX=${PREFIX}" ..
CPATH="$CPATH:$CONDA_PREFIX/include" make -j install
cd ..

# Heritrix 3 - handled in meta.yaml

echo -e " - \e[4mPost-build actions...\e[0m"
# Clean
find . -type d -or -type l | grep /__pycache__$ | xargs -I{} rm -rf {}
find . -type f -or -type l | grep [.]o$ | xargs -I{} rm -rf {}
find . -type d -or -type l | grep /CMakeFiles$ | xargs -I{} rm -rf {}

# Copy src (not usual in builds, but we need src because we are using bash and python scripts)
cp -r "$SRC_DIR/bitextor" "$PREFIX"
cp -r "$SRC_DIR/gopath" "$PREFIX"
cp -r "$SRC_DIR/heritrix-3.4.0-SNAPSHOT" "$PREFIX"

# Soft links (is necessary that is relative, not absolute)
cd "$PREFIX/bin"

ln -s ../gopath/bin/giashard ./giashard || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'giashard'...\e[0m" # Warning: bitextor will look for giashard at ~/go/bin
ln -s "$PREFIX/heritrix-3.4.0-SNAPSHOT" "$PREFIX/heritrix3" || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'heritrix3' (directory)...\e[0m"

# Restore initial working directory (trying to prevent errors)
cd "$SRC_DIR"

echo -e "\e[4m\e[1mBuild done!\e[0m"
