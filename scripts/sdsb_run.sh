find ../../itedata/sdsb-1-0 -name "*.cnf*" -exec ./sbsat '{}' --debug 0 -f -exq -inf -equ -cco 2 -ram 1000 -time 10 --bdd-ordering 11 -ve 1 --stats-file - \;
