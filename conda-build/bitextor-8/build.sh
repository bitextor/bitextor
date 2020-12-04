
echo -e "\e[4m\e[1mBuilding Bitextor...\e[0m"

cd "bitextor"

# Go dependencies
echo -e " - \e[4mInstalling Go dependencies...\e[0m"
mkdir "$PWD/../gopath"
go env
#export CGO_ENABLED="1"
export GOPATH="$PWD/../gopath"
go get github.com/paracrawl/giawarc/...
go get github.com/paracrawl/giashard/...

# Pip dependencies
echo -e " - \e[4mInstalling python3 dependencies...\e[0m"
export PIP_NO_INDEX="False" # We are downloading requisites from PyPi
export PIP_NO_DEPENDENCIES="False" # We need the dependencies from our defined dependencies
pip3 install -r requirements.txt
pip3 install -r bicleaner/requirements.txt
pip3 install https://github.com/kpu/kenlm/archive/master.zip --install-option="--max_order 7"
pip3 install -r bifixer/requirements.txt
### Biroamer
pip3 install -r biroamer/requirements.txt
### Biroamer model
python -m spacy download en_core_web_sm

# CLD3
echo -e " - \e[4mInstalling CLD3...\e[0m"
cd ..
tmp_dir="$(mktemp -d -t cld3-XXXXXXXXXX)"
cwd="$PWD"
cd "$tmp_dir"
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.11.4/protobuf-all-3.11.4.tar.gz
tar -zxvf protobuf-all-3.11.4.tar.gz
mv "protobuf-3.11.4" "$cwd/"
cd "$cwd/protobuf-3.11.4"
mkdir "$cwd/protobuf"
./configure --prefix="$cwd/protobuf"
make
make check
make install || echo -e " - \e[31m\e[4mcld3-install: something wrong happened with 'make install'...\e[0m"
LD_LIBRARY_PATH="$cwd/protobuf/lib" ldconfig -C "$cwd/ld.so.cache" || \
(
  echo -e " - \e[31m\e[4mcld3-install: something wrong happened with 'ldconfig'...\e[0m";
  echo -e " - \e[4mcld3-install: problem at '$cwd/protobuf-3.11.4' (you can try to fix it manually)...\e[0m";
  echo -e " - \e[4mcld3-install: sleep 10800: use 'ps aux | grep sleep' and 'kill -s sigint \$pid' in order to continue...\e[0m";
  sleep 10800;
)
pip3 install Cython
pip3 install pycld3
cd ..
cd bitextor

# Heritrix 3
# Once installed and 'conda activate BITEXTOR_ENV': ln -s $CONDA_PREFIX/bitextor/heritrix-3.4.0-SNAPSHOT/ /opt/heritrix3
#echo -e " - \e[4mInstalling Heritrix3...\e[0m"
#tmp_dir="$(mktemp -d -t heritrix3-XXXXXXXXXX)"
#cwd="$PWD"
#cd "$tmp_dir"
#wget http://builds.archive.org/maven2/org/archive/heritrix/heritrix/3.4.0-SNAPSHOT/heritrix-3.4.0-SNAPSHOT-dist.zip
#unzip heritrix-3.4.0-SNAPSHOT-dist.zip
#mv "heritrix-3.4.0-SNAPSHOT" "$cwd/"
#cd "$cwd/heritrix-3.4.0-SNAPSHOT"
#cd ..

echo -e " - \e[4mChecking and generating files...\e[0m"
# Execute autogen.sh and try to fix known installation issues if any
autogen_output=$(./autogen.sh) && ok="0" || \
  (status=$?; \
  if [[ "$(echo $autogen_output | grep 'tensorflow for Python... no' | wc -l)" != "0" ]]; then \
    echo -e " - \e[4mcheck-gen: trying to fix known installation issue related to TensorFlow...\e[0m"; \
    ok="1"; \
  else \
    echo -e " - \e[31m\e[4mcheck-gen: unknown installation issue happened... (status: $status)\e[0m"; \
    false; \ # Stop execution
  fi)

if [[ "$ok" == "1" ]]; then pip3 uninstall -y tensorflow && pip3 install tensorflow==2.2 keras==2.2.5; fi # tensorflow fix
if [[ "$ok" != "0" ]]; then ./autogen.sh; fi  # Re-execute autogen.sh

echo -e " - \e[4mMake...\e[0m"
make

# Make Biroamer
cwd="$PWD"
cd "biroamer/fast_align" && \
mkdir build && \
cd build && \
cmake .. && \
make -j || \
echo -e " - \e[31m\e[4mmake: could not build fast_align (this might lead to problems with Biroamer)...\e[0m"
cd "$cwd"

echo -e " - \e[4mMaking post-build actions...\e[0m"
# Clean
find . -type d -or -type l | grep /__pycache__$ | xargs -I{} rm -rf {}
find . -type f -or -type l | grep [.]o$ | xargs -I{} rm -rf {}
find . -type d -or -type l | grep /CMakeFiles$ | xargs -I{} rm -rf {}

# Copy src (not usual in builds, but we need src because we are using bash and python scripts)
cp -r "$SRC_DIR/bitextor" "$PREFIX"
cp -r "$SRC_DIR/gopath" "$PREFIX"
cp -r "$SRC_DIR/protobuf" "$PREFIX"

# Soft link (is necessary that is relative, not absolute)
cd "$PREFIX/bin"

### Create bitextor and bitextor.sh; if bitextor fails, try bitextor.sh; complain if both fails
#ln -s ../bitextor/bitextor.sh ./bitextor && \
#ln -s ../bitextor/bitextor.sh ./bitextor.sh || (\
#ln -s ../bitextor/bitextor.sh ./bitextor.sh || \
#echo "Could not create bitextor link...")
### bitextor.sh uses dirname, so we cannot use soft links directly...
### Other soft links
ln -s ../gopath/bin/giawarc ./giawarc || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'giawarc'...\e[0m" # Warning: bitextor will look for giarwarc at ~/go/bin
ln -s ../gopath/bin/giashard ./giashard || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'giashard'...\e[0m" # Warning: bitextor will look for giashard at ~/go/bin
ln -s ../protobuf/bin/protoc ./protoc || echo -e " - \e[31m\e[4mpost-build-act: could not create link 'protoc'...\e[0m"

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
