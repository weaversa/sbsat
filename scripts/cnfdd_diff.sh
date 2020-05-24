#!/bin/sh
#/Users/fett/SAT/sbsat-new/src/sbsat -seed 10 -f -ve 1 -xfs -ges --debug 0 $1 >& /tmp/sbsatxor
#/Users/fett/SAT/sbsat-new/src/sbsat -seed 10 -f -ve 1 --debug 0 $1 >& /tmp/sbsat
/Users/fett/SAT/sbsat-new/src/sbsat -seed 10 -f -ve -1 -cvl 10 -exq -inf -equ -s f --debug 0 $1 >& /tmp/sbsatxor ;
/Users/fett/SAT/sbsat-new/src/sbsat -seed 10 -s p --debug 0 $1 >& /tmp/sbsat ;
diff /tmp/sbsatxor /tmp/sbsat
