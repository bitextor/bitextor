# This file is part of mgiza++.  Its use is licensed under the GNU General
# Public License version 2 or, at your option, any later version.

PRE=test
SRC=fr
TGT=en
NUM=1
SCRIPT_DIR=/opt/AO/sw/edinburgh-code/scripts-20110926-1425

export QMT_HOME=/root/workspace/mgizapp

rm -rf out

$QMT_HOME/scripts/force-align-moses.sh $PRE $SRC $TGT out 1

echo "FINISHED forced alignment"

$SCRIPT_DIR/../merge_alignment.py out/giza-inverse.$NUM/$SRC-$TGT.A3.final.part* | gzip -c > out/giza-inverse.$NUM/$SRC-$TGT.A3.final.gz
$SCRIPT_DIR/../merge_alignment.py out/giza.$NUM/$TGT-$SRC.A3.final.part* | gzip -c > out/giza.$NUM/$TGT-$SRC.A3.final.gz

$SCRIPT_DIR/training/symal/giza2bal.pl -d "gzip -cd out/giza.$NUM/$TGT-$SRC.A3.final.gz" -i "gzip -cd out/giza-inverse.$NUM/$SRC-$TGT.A3.final.gz" | $SCRIPT_DIR/training/symal/symal -alignment="grow" -diagonal="yes" -final="yes" -both="yes" > out/aligned.1.grow-diag-final-and

echo "FINISHED giza2bal & symal"

$SCRIPT_DIR/training/phrase-extract/extract $PRE.$TGT $PRE.$SRC out/aligned.1.grow-diag-final-and out/extract.1 7 orientation --model wbe-msd

