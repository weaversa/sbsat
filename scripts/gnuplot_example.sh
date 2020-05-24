sort -n -k 9 -k 6 snapshot-20111023-2133/f_cc_cco0_2500M_500s/sat-ex.csv > tmp.csv && gnuplot -e "plot \"tmp.csv\" using 6:13 title 'RAM'"

# To remove runs that end in error, awk -F, '{ if ($9 == 0) print $0 }'