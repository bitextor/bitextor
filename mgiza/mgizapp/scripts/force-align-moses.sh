#!/usr/bin/env bash
#
# This file is part of mgiza++.  Its use is licensed under the GNU General
# Public License version 2 or, at your option, any later version.

MGIZA=${QMT_HOME}/bin/mgiza

if [ $# -lt 4 ]; then
	echo "OK, this is simple, put me into your Moses training directory, link your source/target corpus" 1>&2
	echo "and run " $0 " PREFIX src_tag tgt_tag root-dir." 1>&2
	echo "and get force-aligned data: root-dir/giza.[src-tgt|tgt-src]/*.A3.final.* " 1>&2
	echo "make sure I can find PREFIX.src_tag-tgt_tag and PREFIX.tgt_tag-src_tag, and \${QMT_HOME} is set" 1>&2
	exit
fi

PRE=$1
SRC=$2
TGT=$3
ROOT=$4
NUM=$5

mkdir -p $ROOT/giza-inverse.${NUM}
mkdir -p $ROOT/giza.${NUM}
mkdir -p $ROOT/prepared.${NUM}

echo "Generating corpus file " 1>&2

${QMT_HOME}/scripts/plain2snt-hasvcb.py prepared.${NUM}/$SRC.vcb prepared.${NUM}/$TGT.vcb ${PRE}.${SRC} ${PRE}.${TGT} $ROOT/prepared.${NUM}/${TGT}-${SRC}.snt $ROOT/prepared.${NUM}/${SRC}-${TGT}.snt $ROOT/prepared.${NUM}/$SRC.vcb $ROOT/prepared.${NUM}/$TGT.vcb

ln -sf $PWD/prepared.${NUM}/$SRC.vcb.classes $PWD/prepared.${NUM}/$TGT.vcb.classes $ROOT/prepared.${NUM}/

echo "Generating co-occurrence file " 1>&2

${QMT_HOME}/bin/snt2cooc $ROOT/giza.${NUM}/$TGT-${SRC}.cooc $ROOT/prepared.${NUM}/$SRC.vcb $ROOT/prepared.${NUM}/$TGT.vcb $ROOT/prepared.${NUM}/${TGT}-${SRC}.snt
${QMT_HOME}/bin//snt2cooc $ROOT/giza-inverse.${NUM}/$SRC-${TGT}.cooc $ROOT/prepared.${NUM}/$TGT.vcb $ROOT/prepared.${NUM}/$SRC.vcb $ROOT/prepared.${NUM}/${SRC}-${TGT}.snt

echo "Running force alignment " 1>&2

$MGIZA giza.$TGT-$SRC/$TGT-$SRC.gizacfg -c $ROOT/prepared.${NUM}/$TGT-$SRC.snt -o $ROOT/giza.${NUM}/$TGT-${SRC} \
-s $ROOT/prepared.${NUM}/$SRC.vcb -t $ROOT/prepared.${NUM}/$TGT.vcb -m1 0 -m2 0 -mh 0 -coocurrence $ROOT/giza.${NUM}/$TGT-${SRC}.cooc \
-restart 11 -previoust giza.$TGT-$SRC/$TGT-$SRC.t3.final \
-previousa giza.$TGT-$SRC/$TGT-$SRC.a3.final -previousd giza.$TGT-$SRC/$TGT-$SRC.d3.final \
-previousn giza.$TGT-$SRC/$TGT-$SRC.n3.final -previousd4 giza.$TGT-$SRC/$TGT-$SRC.d4.final \
-previousd42 giza.$TGT-$SRC/$TGT-$SRC.D4.final -m3 0 -m4 1

$MGIZA giza.$SRC-$TGT/$SRC-$TGT.gizacfg -c $ROOT/prepared.${NUM}/$SRC-$TGT.snt -o $ROOT/giza-inverse.${NUM}/$SRC-${TGT} \
-s $ROOT/prepared.${NUM}/$TGT.vcb -t $ROOT/prepared.${NUM}/$SRC.vcb -m1 0 -m2 0 -mh 0 -coocurrence $ROOT/giza-inverse.${NUM}/$SRC-${TGT}.cooc \
-restart 11 -previoust giza.$SRC-$TGT/$SRC-$TGT.t3.final \
-previousa giza.$SRC-$TGT/$SRC-$TGT.a3.final -previousd giza.$SRC-$TGT/$SRC-$TGT.d3.final \
-previousn giza.$SRC-$TGT/$SRC-$TGT.n3.final -previousd4 giza.$SRC-$TGT/$SRC-$TGT.d4.final \
-previousd42 giza.$SRC-$TGT/$SRC-$TGT.D4.final -m3 0 -m4 1

