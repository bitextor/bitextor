# Conda build

Instructions in order to create, install and upload a conda package.

You will need to [download](https://docs.conda.io/en/latest/miniconda.html) and [install](https://conda.io/projects/conda/en/latest/user-guide/install/index.html) Miniconda.

## Create package

First, you will need to install some dependencies:

```bash
conda install -y -c conda-forge conda-build conda-verify
```

Once the dependencies are installed, we can create the package:

```bash
CONDA_CHANNELS_BITEXTOR="-c conda-forge -c bioconda -c dmnapolitano -c esarrias"
CONDA_RELEASE_BITEXTOR="bitextor" # Change to "nightly" in order to build nightly version

# BEGIN tmp solution
GIT_DESCRIBE=$(git describe --always 2> /dev/null)
OWN_DATE=$(date +"%Y%m%d%H%M")

if [[ "$GIT_DESCRIBE" != "" ]]; then
  export OWN_GIT_BUILD_STR="${OWN_DATE}_${GIT_DESCRIBE}"
else
  export OWN_GIT_BUILD_STR="${OWN_DATE}"
fi
# END tmp solution

conda build --no-anaconda-upload --no-test $CONDA_CHANNELS_BITEXTOR "./conda_build/$CONDA_RELEASE_BITEXTOR"
```

When the build finishes, the path to the package should be printed in the log. The path to the package should have the following structure:

```
${CONDA_PREFIX}/conda-bld/linux-64/bitextor-{version}-{date}_{commit}.tar.bz2
```

If you install the generated package (i.e. local installation), the dependencies will not be installed. Be aware that, despite install the dependencies, the constraints might be ignored, what might lead to an inconsistent installation (this does not happen when the package is [uploaded](#upload-package) to the [anaconda repo](https://anaconda.org/anaconda/repo)). Run:

```bash
# Install local package
conda install -y /path/to/package

CONDA_PACKAGE_NAME_BITEXTOR="bitextor" # Change to "bitextor-nightly" if you built the nightly version

# Install dependencies
conda update -y --only-deps $CONDA_CHANNELS_BITEXTOR $CONDA_PACKAGE_NAME_BITEXTOR
```

## Upload package

If you want to upload your package, install:

```bash
conda install -y -c conda-forge anaconda-client
```

In order to upload the package, we just need to execute the following command and follow the instructions:

```bash
anaconda upload /path/to/package
```
