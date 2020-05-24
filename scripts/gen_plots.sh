#!/bin/sh

GNU_PLOT_OPTS="set term postscript eps color \"Helvetica\" 10"

NUMCLAUSES=2
RAM=6
NUMBDDS=8
TIME=13

SNAPS=`find ../data -type d -name "snapshot-*"`

for SNAP in $SNAPS
do
  SBSAT_CMDLN=`find $SNAP -type d -name "*_*"`
  CLAUSES_BDDS_PLOTS=""
  RAM_TIME_PLOTS=""
  RAM_PLOTS=""
  TIME_PLOTS=""
  for DIR in $SBSAT_CMDLN
  do
    SBSAT_SHORT_DESC=${DIR##*/} # remove all the prefix until "/" character
    SBSAT_DESC=`cat $DIR/desc.txt`
    
    GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/$SBSAT_SHORT_DESC.CLAUSES_BDDS.ps\""
    CLAUSES_BDDS_PLOT="\"/tmp/$SBSAT_SHORT_DESC.CLAUSES_BDDS.csv\" using $NUMCLAUSES:$NUMBDDS title 'Number of Clauses vs. Number of BDDs -- $SBSAT_DESC'"
    cat $DIR/*.csv | awk -F, '{ if ($9 == 0) print $0 }' > /tmp/$SBSAT_SHORT_DESC.CLAUSES_BDDS.csv && gnuplot -e "$GP_TERM; plot $CLAUSES_BDDS_PLOT"
    CLAUSES_BDDS_PLOTS="$CLAUSES_BDDS_PLOTS $CLAUSES_BDDS_PLOT,"

    GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/$SBSAT_SHORT_DESC.RAM_TIME.ps\""
    RAM_TIME_PLOT="\"/tmp/$SBSAT_SHORT_DESC.RAM_TIME.csv\" using $RAM:$TIME title 'RAM Usage vs. Runtime -- $SBSAT_DESC'"
    cat $DIR/*.csv | awk -F, '{ if ($9 == 0) print $0 }' > /tmp/$SBSAT_SHORT_DESC.RAM_TIME.csv && gnuplot -e "$GP_TERM; plot $RAM_TIME_PLOT"
    RAM_TIME_PLOTS="$RAM_TIME_PLOTS $RAM_TIME_PLOT,"

    GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/$SBSAT_SHORT_DESC.RAM.ps\""
    RAM_PLOT="\"/tmp/$SBSAT_SHORT_DESC.RAM.csv\" using $RAM title 'RAM Usage -- $SBSAT_DESC'"
    sort -n -k 9 -k $RAM $DIR/*.csv | awk -F, '{ if ($9 == 0) print $0 }' > /tmp/$SBSAT_SHORT_DESC.RAM.csv && gnuplot -e "$GP_TERM; plot $RAM_PLOT"
    RAM_PLOTS="$RAM_PLOTS $RAM_PLOT,"

    GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/$SBSAT_SHORT_DESC.TIME.ps\""
    TIME_PLOT="\"/tmp/$SBSAT_SHORT_DESC.TIME.csv\" using $TIME title 'Runtime -- $SBSAT_DESC'"
    sort -n -k 9 -k $TIME $DIR/*.csv | awk -F, '{ if ($9 == 0) print $0 }' > /tmp/$SBSAT_SHORT_DESC.TIME.csv && gnuplot -e "$GP_TERM; plot $TIME_PLOT"
    TIME_PLOTS="$TIME_PLOTS $TIME_PLOT,"

    BENCHES=`find $DIR -name "*.csv"`
    for BENCH in $BENCHES
    do
      BENCH_NAME=${BENCH##*/}
#      echo $BENCH_NAME
    done
  done

  CLAUSES_BDDS_PLOTS=${CLAUSES_BDDS_PLOTS%?}
  GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/all.CLAUSES_BDDS.ps\""
  gnuplot -e "$GP_TERM; plot $CLAUSES_BDDS_PLOTS"

  RAM_TIME_PLOTS=${RAM_TIME_PLOTS%?}
  GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/all.RAM_TIME.ps\""
  gnuplot -e "$GP_TERM; plot $RAM_TIME_PLOTS"

  RAM_PLOTS=${RAM_PLOTS%?}
  GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/all.RAM.ps\""
  gnuplot -e "$GP_TERM; plot $RAM_PLOTS"

  TIME_PLOTS=${TIME_PLOTS%?}
  GP_TERM="$GNU_PLOT_OPTS; set output \"$SNAP/all.TIME.ps\""
  gnuplot -e "$GP_TERM; plot $TIME_PLOTS"

done

rm -rf /tmp/*.csv
