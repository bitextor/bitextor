
echo -e "\e[4m\e[1mBuilding Bitextor...\e[0m"

cd "bitextor"

# Go dependencies
echo -e " - \e[4mInstalling Go dependencies...\e[0m"
mkdir "$PWD/../gopath"
go env
#export CGO_ENABLED="1"
export GOPATH="$PWD/../gopath"
go get github.com/paracrawl/giashard/...

# Pip dependencies
echo -e " - \e[4mInstalling python3 dependencies...\e[0m"
export PIP_NO_INDEX="False" # We are downloading requisites from PyPi
export PIP_NO_DEPENDENCIES="False" # We need the dependencies from our defined dependencies
pip3 install -r requirements.txt
pip3 install -r bicleaner/requirements.txt
pip3 install https://github.com/kpu/kenlm/archive/master.zip --install-option="--max_order 7"
pip3 install -r bifixer/requirements.txt
### CLD3
pip3 install Cython
pip3 install pycld3
### Biroamer and model
pip3 install -r biroamer/requirements.txt && \
python -m spacy download en_core_web_sm && \
pip3 show spacy && pip3 show en_core_web_sm && \
pip3 uninstall -y spacy && pip3 install spacy==2.2.3 && \
pip3 show spacy && pip3 show en_core_web_sm
### Linguacrawl
pip3 install git+https://github.com/transducens/linguacrawl.git || echo " - \e[4mcheck-gen: linguacrawl installation failed...\e[0m"

echo -e " - \e[4mMake...\e[0m"
mkdir -p build && cd build
CPATH="$CPATH:$CONDA_PREFIX/include" cmake ..
CPATH="$CPATH:$CONDA_PREFIX/include" make -j
cd ..
cp warc2text/cld2/internal/libcld2.so $PREFIX/lib

# Heritrix 3
# echo -e " - \e[4mInstalling Heritrix3...\e[0m"
# download and unzipping handled by source property in meta.yaml

echo -e " - \e[4mMaking post-build actions...\e[0m"
# Clean
find . -type d -or -type l | grep /__pycache__$ | xargs -I{} rm -rf {}
find . -type f -or -type l | grep [.]o$ | xargs -I{} rm -rf {}
find . -type d -or -type l | grep /CMakeFiles$ | xargs -I{} rm -rf {}

# Copy src (not usual in builds, but we need src because we are using bash and python scripts)
cp -r "$SRC_DIR/bitextor" "$PREFIX"
cp -r "$SRC_DIR/gopath" "$PREFIX"
cp -r "$SRC_DIR/heritrix-3.4.0-SNAPSHOT" "$PREFIX"

# Soft link (is necessary that is relative, not absolute)
cd "$PREFIX/bin"

### Create bitextor and bitextor.sh; if bitextor fails, try bitextor.sh; complain if both fails
#ln -s ../bitextor/bitextor.sh ./bitextor && \
#ln -s ../bitextor/bitextor.sh ./bitextor.sh || (\
#ln -s ../bitextor/bitextor.sh ./bitextor.sh || \
#echo "Could not create bitextor link...")
### bitextor.sh uses dirname, so we cannot use soft links directly...
### Other soft links
ln -s ../gopath/bin/giashard ./giashard || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'giashard'...\e[0m" # Warning: bitextor will look for giashard at ~/go/bin
ln -s "$PREFIX/heritrix-3.4.0-SNAPSHOT" "$PREFIX/heritrix3" || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'heritrix3' (directory)...\e[0m"

### Create an script manually and store it in order to wrap the true call
script="/bin/sh\n\n\$(dirname \"\$0\")/../bitextor/bitextor.sh \"\$@\""
error="0"
scripts=( "./bitextor" "./bitextor.sh" )

for f in ${scripts[@]}; do
  if [[ ! -f "$f" ]]; then
    echo -n "#" >> "$f" && \
    echo -n "!" >> "$f" && \
    echo -e "$script" >> "$f" && \
    chmod 775 "$f" && \
    echo -e " - \e[4mpost-build-act: link '$(basename $f)' created...\e[0m" || \
    echo -e " - \e[31m\e[4mpost-build-act: something wrong happened... the link '$f' might have not been created...\e[0m"
  fi
  if [[ ! -f "$f" ]]; then
    error=$(echo "$error+1" | bc)
    echo -e " - \e[31m\e[4mpost-build-act: could not create link '$f'...\e[0m"
  fi
done

if [[ "$error" == "${#scripts[@]}" ]]; then
  echo -e " - \e[31m\e[4mpost-build-act: could not create any bitextor link[s]...\e[0m"
fi

# Restore initial working directory (trying to prevent errors)
cd "$SRC_DIR"

echo -e "\e[4m\e[1mBuild done!\e[0m"
