#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

usage()
{
  echo "$(basename $0) [-h] [-e <CONDA_ENV_NAME>] [-r] -n <package_name>"
  echo ""
  echo "OPTIONS:"
  echo "  -e <CONDA_ENV_NAME>     Name of the conda environment which will be used to build"
  echo "                          Bitextor. If not specified, the default value is"
  echo "                          the provided package name with the suffix '-build'."
  echo "  -r                      It removes the conda environment before start if exists."
  echo "  -h                      It displays this help message."
  echo "  -n                      Conda package name in order to retrieve the pkg path"
}

CONDA_ENV_NAME=""
CONDA_PACKAGE_NAME=""
REMOVE_ENV_IF_EXISTS=""

while getopts "e:n:rh" options
do
  case "${options}" in
    e) CONDA_ENV_NAME=$OPTARG;;
    r) REMOVE_ENV_IF_EXISTS="y";;
    h) usage
       exit 0;;
    n) CONDA_PACKAGE_NAME=$OPTARG;;
    \?) usage 1>&2
        exit 1;;
  esac
done

if [[ "$CONDA_PACKAGE_NAME" == "" ]]; then
  >&2 echo "Error: package name is mandatory"
  >&2 echo ""
  usage 1>&2
  exit 1
fi

if [[ "$CONDA_ENV_NAME" == "" ]]; then
  CONDA_ENV_NAME="${CONDA_PACKAGE_NAME}-build"
fi

CONDA_PACKAGE_PATH="${SCRIPT_DIR}/${CONDA_PACKAGE_NAME}"

if [[ ! -f "${CONDA_PACKAGE_PATH}/meta.yaml" ]]; then
  >&2 echo "File 'meta.yaml' not found: ${CONDA_PACKAGE_PATH}/meta.yaml"
  exit 1
fi

source $(conda info --base)/etc/profile.d/conda.sh

if [[ "$REMOVE_ENV_IF_EXISTS" == "y" ]]; then
  conda remove -y -n $CONDA_ENV_NAME --all
fi

if [[ "$(conda env list | grep ^$CONDA_ENV_NAME[\ +])" != "" ]]; then
  >&2 echo "Error: conda environment '$CONDA_ENV_NAME' already exists"
  >&2 echo ""
  usage 1>&2
  exit 1
fi

conda create -y -n $CONDA_ENV_NAME -c conda-forge conda-build conda-verify
conda activate $CONDA_ENV_NAME

echo "git describe --tags:"
git describe --tags || true

echo "git describe --always:"
git describe --always

CONDA_CHANNELS="-c conda-forge -c bioconda -c dmnapolitano -c esarrias"

# Make build
conda-build --no-anaconda-upload --no-test $CONDA_CHANNELS $CONDA_PACKAGE_PATH
