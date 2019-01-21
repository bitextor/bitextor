#!/bin/bash

set -o pipefail

exit_program()

{
  >&2 echo "bitextor -s <SNAKEFILE> [-j <NUMJOBS>] [-c <CLUSTERCOMMAND>] [-g <CLUSTERCONFIG>]"
  >&2 echo ""
  >&2 echo "Bitextor is implemented using the tool Snakemake. It requires to provide a Snakemake"
  >&2 echo "configuration file (SNAKEFILE)."
  >&2 echo ""
  >&2 echo "OPTIONS:"
  >&2 echo "  -s <SNAKEFILE>          SNAKEFILE is a configuration file with all the parameters"
  >&2 echo "                          required to run the pipeline."
  >&2 echo "  -j <NUMJOBS>            NUMJOBS is the number of jobs that can be run in parallel."
  >&2 echo "                          If option -c (cluster) is enabled, each job is submitted"
  >&2 echo "                          to different nodes."
  >&2 echo "  -c <CLUSTERCOMMAND>     Command used to submit jobs to the cluster (for example"
  >&2 echo "                          qsub in PBS or sbatch in SLURM."
  >&2 echo "  -g <CLUSTERCONFIG>      Config file for the clsuter. This allows to set specific"
  >&2 echo "                          options for some rules in snakemake."
  exit 1
}


SNAKEFILE=""
NUMJOBS=1
CLUSTERCOMMAND=""
CLUSTERCONFIG=""

if [[ $(command -v snakemake | wc -l) -eq 0 ]]; then
  >&2 echo "Bitextor cannot be run if the tool snakemake is not installed in the system";
  exit -1
fi

ARGS=$(getopt -o hs:j:c:g: -- "$@")
eval set -- $ARGS
for i
do
  case "$i" in
    -s )
      shift
      SNAKEFILE="$1"
      shift
      ;;
    -j )
      shift
      NUMJOBS="$1"
      shift
      ;;
    -c )
      shift
      CLUSTERCOMMAND="--cluster $1"
      shift
      ;;
    -g )
      shift
      CLUSTERCONFIG="--cluster-config $1"
      shift
      ;;
    -h | --help)
      exit_program $(basename $0)
      ;;
    --)
      shift
      break
      ;;
  esac
done

if [ "$SNAKEFILE" == "" ]; then
  >&2 echo "Argument -s <SNAKEFILE> is mandatory. Please, specify a snakemake configuration file.";
  exit -1
fi

snakemake --snakefile "$(dirname $0)/snakemake/Snakefile" --configfile $SNAKEFILE -j $NUMJOBS $CLUSTERCOMMAND $CLUSTERCONFIG

rm -rf $BUILDDICTTMP
