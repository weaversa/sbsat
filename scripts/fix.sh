#!/bin/sh

NUM_CLAUSES=2
RAM=6
NUM_BDDS=8
ERROR=9


# CNF_COMPRESSION=12
# CNF_TIME=13
# TOTAL_TIME=14

CNF_TIME=12
TOTAL_TIME=13


MEM_ERR=105
TIME_ERR=107


BENCH=$*

echo $BENCH

cat $BENCH.csv | awk -v ERROR=$ERROR -F, '{ if ($ERROR == 0) print $0 }' > ${BENCH}_errors_removed.csv
cut -d , -f $RAM ${BENCH}_errors_removed.csv | sort -n -k 1 > ${BENCH}_ram_sorted.csv
# cut -d , -f $CNF_COMPRESSION ${BENCH}_errors_removed.csv | sort -n -k 1 > ${BENCH}_cnf_compression_sorted.csv
cut -d , -f $CNF_TIME ${BENCH}_errors_removed.csv | sort -n -k 1 > ${BENCH}_cnf_time_sorted.csv
cut -d , -f $TOTAL_TIME ${BENCH}_errors_removed.csv | sort -n -k 1 > ${BENCH}_total_time_sorted.csv
cut -d , -f $NUM_BDDS ${BENCH}_errors_removed.csv | sort -n -k 1 > ${BENCH}_num_bdds_sorted.csv
awk -v CNF_TIME=$CNF_TIME '{sum=sum+$CNF_TIME} END {printf("%4.3f\n", sum)}' < ${BENCH}_errors_removed.csv > ${BENCH}_cnf_time.cnt
awk -v TOTAL_TIME=$TOTAL_TIME '{sum=sum+$TOTAL_TIME} END {printf("%4.3f\n", sum)}' < ${BENCH}_errors_removed.csv > ${BENCH}_total_time.cnt
# awk -v CNF_COMPRESSION=$CNF_COMPRESSION '{sum=sum+$CNF_COMPRESSION} END {printf("%4.3f\n", sum)}' < ${BENCH}_errors_removed.csv > ${BENCH}_total_compression.cnt
