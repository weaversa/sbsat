while ((1));
do
  ../../fuzzsat-0.1/fuzzsat -I 10 -i 8 -r 3 -L 6 -t -m > /tmp/xortest.cnf;
  ##./sbsat -s f --debug 0 -ve 0 /tmp/test.cnf >& /tmp/func ;
  ##./sbsat -s p --debug 0 -ve 0 /tmp/test.cnf >& /tmp/pico ;
  ##./sbsat -s s --precompute-smurfs 1 --debug 0 -f -ve 0 -inf -equ -exq --limit-all 1000 /tmp/test.cnf >& /tmp/smurf ;
  ##./sbsat -s s --precompute-smurfs 1 --debug 0 -f -ve 0 -inf -equ -exq --limit-and-equ 1000 --limit-minmax 1000 --limit-neg-minmax 1000 /tmp/test.cnf >& /tmp/smurf_with_or_and_xor ;
  ##../../sbsat/src/generator/gentest xor 21 22 8 2 0 | sbsat -All 0 -c -In 0 -Eq 0 --debug 0 > xortest.cnf ;
  ##./sbsat -seed 10 -f -ve 1 -seed 1 -ges -xfs --debug 0 /tmp/xortest.cnf >& /tmp/sbsatxor2 ;
  ##./sbsat -seed 10 -f -ve 1 -seed 1 --debug 0 /tmp/xortest.cnf >& /tmp/sbsat2 ;
  ./sbsat -seed 10 -f -ve -1 -cvl 10 -exq -inf -equ -s f --debug 0 /tmp/xortest.cnf >& /tmp/sbsatxor2 ;
  ./sbsat -seed 10 -f -ve -1 -cvl 10 -exq -inf -equ -s p --debug 0 /tmp/xortest.cnf >& /tmp/sbsat2 ;
  diff /tmp/sbsatxor2 /tmp/sbsat2
  OUT=$?
  if [ $OUT -eq 0 ];then
      echo -n "*";
  else
      echo "sbsatxor and sbsat outputs differ!"
      break
  fi
done
