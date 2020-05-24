#!/bin/bash
while ((1));
##do //comment to break it
## This doesn't really work anymore because the return value of SBSAT is not dependent upon the SAT/UNSAT result.
  ../../fuzzsat-0.1/fuzzsat -I 100 > /tmp/test.cnf;
  ./sbsat --limit-all 1000 -seed 100 --precompute-smurfs 0 --debug 0 -inf -ve -1 -cvl 12 -f /tmp/test.cnf;
  SB_RESULT1000=$?;
  #if [ $SB_RESULT1000 != 0 ]; then break; fi;
  ./sbsat --limit-all 0 -seed 100 --precompute-smurfs 0 --debug 0 -inf -ve -1 -cvl 12 -f /tmp/test.cnf;
  SB_RESULT0=$?;
  #if [ $SB_RESULT0 != 0 ]; then; fi;
  ./sbsat --limit-all 1000 -seed 100 --precompute-smurfs 0 --debug 0 -inf -ve 0 /tmp/test.cnf;
  FS_RESULT=$?;
  #if [ $FS_RESULT != 0 ]; then break; fi;
  if [ $FS_RESULT != $SB_RESULT0 ]; then exit 10; fi;
  if [ $FS_RESULT != $SB_RESULT1000 ]; then exit 10; fi;
  echo "------"
done