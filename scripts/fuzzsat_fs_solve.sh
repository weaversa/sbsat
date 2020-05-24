#!/bin/bash

while ((1));
do
  ../../fuzzsat-0.1/fuzzsat -I 10 > /tmp/test.cnf;
  ./sbsat --limit-all 1000 --precompute-smurfs 0 --debug 0 -f -inf -equ -cco 2 -ram 1500 -ve -1 -cvl 12 -ccl 10 /tmp/test.cnf;
  SB_RESULT1000=$?;
  if [ $SB_RESULT1000 != 0 ]; then break; fi;
#  ./sbsat --limit-all 0 --precompute-smurfs 0 --debug 0 -inf -equ -cco 2 -ram 1500 -ve -1 -cvl 12 -ccl 10 /tmp/test.cnf;
#  SB_RESULT0=$?;
#  if [ $SB_RESULT0 != 0 ]; then break; fi;
  ./sbsat --limit-all 1000 --precompute-smurfs 0 --debug 0 -inf -ram 1500 -ve 0 /tmp/test.cnf;
  FS_RESULT=$?;
  if [ $FS_RESULT != 0 ]; then break; fi;
#  if [ $FS_RESULT != $SB_RESULT0 ]; then break; fi;
  if [ $FS_RESULT != $SB_RESULT1000 ]; then break; fi;
done
