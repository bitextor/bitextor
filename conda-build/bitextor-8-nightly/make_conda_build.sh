#!/bin/bash

usage()
{
  echo "make_conda_build [-h] [-e <CONDA_ENV_NAME>] [-r]"
  echo ""
  echo "OPTIONS:"
  echo "  -e <CONDA_ENV_NAME>     Name of the conda environment which will be used to build"
  echo "                          Bitextor. If not specified, the default value is"
  echo "                          \"bitextor-build\"."
  echo "  -r                      It removes the conda environment before start if exists."
  echo "  -h                      It displays this help message."
}

CONDA_ENV_NAME="bitextor-build-nightly"
REMOVE_ENV_IF_EXISTS=""

while getopts "e:rh" options
do
  case "${options}" in
    e) CONDA_ENV_NAME=$OPTARG;;
    r) REMOVE_ENV_IF_EXISTS="y";;
    h) usage
       exit 0;;
    \?) usage 1>&2
        exit 1;;
  esac
done

source $(conda info --base)/etc/profile.d/conda.sh

if [[ "$REMOVE_ENV_IF_EXISTS" == "y" ]]; then
  conda remove -y -n $CONDA_ENV_NAME --all
fi

if [[ "$(conda env list | grep ^$CONDA_ENV_NAME[\ +])" != "" ]]; then
  echo "Error: conda environment '$CONDA_ENV_NAME' already exists"
  echo
  usage 1>&2
  exit 1
fi

conda create -y -n $CONDA_ENV_NAME python=3.8.5
conda activate $CONDA_ENV_NAME

if [[ "$(conda config --show channels | grep conda-forge)" == "" ]]; then
  echo "Info: adding 'conda-forge' channel (remove it manually after the build is done if you want). This is common at fresh installations"
  conda config --add channels conda-forge
fi
if [[ "$(conda config --show channels | grep bioconda)" == "" ]]; then
  echo "Info: appending 'bioconda' channel (remove it manually after the build is done if you want)"
  conda config --append channels bioconda # snakemake
fi
if [[ "$(conda config --show channels | grep dmnapolitano)" == "" ]]; then
  echo "Info: appending 'dmnapolitano' channel (remove it manually after the build is done if you want)"
  conda config --append channels dmnapolitano # warcio
fi
if [[ "$(conda config --show channels | grep esarrias)" == "" ]]; then
  echo "Info: appending 'esarrias' channel (remove it manually after the build is done if you want)"
  conda config --append channels esarrias # uchardet
fi

conda config --show channels
conda install -y conda-build
conda install -y conda-verify

echo "git describe --tags:"
git describe --tags || true

echo "git describe --always:"
git describe --always

GIT_DESCRIBE=$(git describe --always 2> /dev/null)

export OWN_DATE=$(date +"%Y%m%d%H%M")

if [[ "$GIT_DESCRIBE" != "" ]]; then
  export OWN_GIT_BUILD_STR="${GIT_DESCRIBE}"
else
  export OWN_GIT_BUILD_STR="${OWN_DATE}"
fi

export OWN_GIT_DESCRIBE_NUMBER="0"

echo "--------- Info ---------"
echo "$GIT_DESCRIBE"
echo "$OWN_GIT_BUILD_STR"
echo "$OWN_GIT_DESCRIBE_NUMBER"
echo "$OWN_DATE"
echo "------------------------"

if [[ -f ./meta.yaml ]]; then
  conda-build --no-anaconda-upload .
else
  echo "File meta.yaml not found on current directory. Go to build directory and execute this script again."
fi