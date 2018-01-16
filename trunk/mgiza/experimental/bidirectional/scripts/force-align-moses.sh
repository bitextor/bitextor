#!/usr/bin/env bash

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

mkdir -p $ROOT/giza.${SRC}-${TGT}
mkdir -p $ROOT/giza.${TGT}-${SRC}
mkdir -p $ROOT/corpus

echo "Generating corpus file " 1>&2

${QMT_HOME}/scripts/plain2snt-hasvcb.py corpus/$SRC.vcb corpus/$TGT.vcb ${PRE}.${SRC} ${PRE}.${TGT} $ROOT/corpus/${TGT}-${SRC}.snt $ROOT/corpus/${SRC}-${TGT}.snt $ROOT/corpus/$SRC.vcb $ROOT/corpus/$TGT.vcb

ln -sf $PWD/corpus/$SRC.vcb.classes $PWD/corpus/$TGT.vcb.classes $ROOT/corpus/

echo "Generating co-occurrence file " 1>&2

${QMT_HOME}/bin/snt2cooc $ROOT/giza.${TGT}-${SRC}/$TGT-${SRC}.cooc $ROOT/corpus/$SRC.vcb $ROOT/corpus/$TGT.vcb $ROOT/corpus/${TGT}-${SRC}.snt
${QMT_HOME}/bin//snt2cooc $ROOT/giza.${SRC}-${TGT}/$SRC-${TGT}.cooc $ROOT/corpus/$TGT.vcb $ROOT/corpus/$SRC.vcb $ROOT/corpus/${SRC}-${TGT}.snt

echo "Running force alignment " 1>&2

$MGIZA giza.$TGT-$SRC/$TGT-$SRC.gizacfg -c $ROOT/corpus/$TGT-$SRC.snt -o $ROOT/giza.${TGT}-${SRC}/$TGT-${SRC} \
-s $ROOT/corpus/$SRC.vcb -t $ROOT/corpus/$TGT.vcb -m1 0 -m2 0 -mh 0 -coocurrence $ROOT/giza.${TGT}-${SRC}/$TGT-${SRC}.cooc \
-restart 11 -previoust giza.$TGT-$SRC/$TGT-$SRC.t3.final \
-previousa giza.$TGT-$SRC/$TGT-$SRC.a3.final -previousd giza.$TGT-$SRC/$TGT-$SRC.d3.final \
-previousn giza.$TGT-$SRC/$TGT-$SRC.n3.final -previousd4 giza.$TGT-$SRC/$TGT-$SRC.d4.final \
-previousd42 giza.$TGT-$SRC/$TGT-$SRC.D4.final -m3 0 -m4 1

$MGIZA giza.$SRC-$TGT/$SRC-$TGT.gizacfg -c $ROOT/corpus/$SRC-$TGT.snt -o $ROOT/giza.${SRC}-${TGT}/$SRC-${TGT} \
-s $ROOT/corpus/$TGT.vcb -t $ROOT/corpus/$SRC.vcb -m1 0 -m2 0 -mh 0 -coocurrence $ROOT/giza.${SRC}-${TGT}/$SRC-${TGT}.cooc \
-restart 11 -previoust giza.$SRC-$TGT/$SRC-$TGT.t3.final \
-previousa giza.$SRC-$TGT/$SRC-$TGT.a3.final -previousd giza.$SRC-$TGT/$SRC-$TGT.d3.final \
-previousn giza.$SRC-$TGT/$SRC-$TGT.n3.final -previousd4 giza.$SRC-$TGT/$SRC-$TGT.d4.final \
-previousd42 giza.$SRC-$TGT/$SRC-$TGT.D4.final -m3 0 -m4 1

