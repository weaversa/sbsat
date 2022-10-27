/***************************************************************************
Copyright (c) 2009, Armin Biere, Johannes Kepler University.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
***************************************************************************/

#include <sbsat.h>

uint8_t process_aiger(BDDManager *BM) {
  uint8_t ret = NO_ERROR;
  intmax_t rhs[2];

  aiger *src = aiger_init ();
  static const char *err;
  err = aiger_read_from_file(src, finputfile);
  if(err) {
    fprintf(stderr, "%s\n", err);
    return ERR_IO_READ;
  }

  d6_printf6("read AIG MILOA %u %u %u %u %u\n", src->maxvar, src->num_inputs, src->num_latches, src->num_outputs, src->num_ands);

  create_all_syms(0, src->maxvar+1);
  
  for(uintmax_t i = 0; i < src->num_ands; i++) {
    aiger_and *a = src->ands + i;
    intmax_t lhs;
    lhs = aiger_lit2var(a->lhs);
    rhs[0] = aiger_lit2var(a->rhs0);
    rhs[1] = aiger_lit2var(a->rhs1);
    
    if(aiger_sign(a->lhs)) {
      rhs[0] = aiger_sign(a->rhs0)?rhs[0]:-rhs[0];
      rhs[1] = aiger_sign(a->rhs1)?rhs[1]:-rhs[1];
      ret = add_or_equals_BDD(BM, lhs, rhs, 2);
    } else {
      rhs[0] = aiger_sign(a->rhs0)?-rhs[0]:rhs[0];
      rhs[1] = aiger_sign(a->rhs1)?-rhs[1]:rhs[1];
      ret = add_and_equals_BDD(BM, lhs, rhs, 2);
    }
    if(ret != NO_ERROR) return ret;
  }

  if(count_BDD_solutions_flag) {
    BM->ExQuantProtectVars = 1;
    //Want to protect inputs
    for(intmax_t i = src->maxvar; i >= 0; i--) {
      //Initialize all vars as unprotected
      uintmax_t_list_add(&BM->ExQuantProtectedVars, i, 0);
    }
    for(uintmax_t i = 0; i < src->num_inputs; i++) {
      //Protect the inputs
      assert(aiger_lit2var(src->inputs[i].lit) <= src->maxvar);
      uintmax_t_list_add(&BM->ExQuantProtectedVars, aiger_lit2var(src->inputs[i].lit), 1);
      d9_printf2("protecting input %u\n", src->inputs[i].lit);
    }
    /*
    for(uintmax_t i = 0; i < src->num_outputs; i++) {
      //Protect the outputs
      assert(aiger_lit2var(src->outputs[i].lit) <= src->maxvar);
      uintmax_t_list_add(&BM->ExQuantProtectedVars, aiger_lit2var(src->outputs[i].lit), 1);
      fprintf(stdout, "protecting output %jd\n", src->outputs[i].lit);
    }
    */
    //Force the outputs to False - SEAN!!! Is this correct to do?
    for(uintmax_t i = 0; i < src->num_outputs; i++) {
      if(src->outputs[i].lit < 2) continue; //constants
      rhs[0] = aiger_lit2var(src->outputs[i].lit);
      rhs[0] = aiger_sign(src->outputs[i].lit)?-rhs[0]:rhs[0];
      ret = add_or_BDD(BM, rhs, 1);
    }
  } else {
    //Force the outputs to False - SEAN!!! Is this correct to do?
    for(uintmax_t i = 0; i < src->num_outputs; i++) {
      rhs[0] = aiger_lit2var(src->outputs[i].lit);
      rhs[0] = aiger_sign(src->outputs[i].lit)?-rhs[0]:rhs[0];
      ret = add_or_BDD(BM, rhs, 1);
    }
  }

  //for (j = 0; j < src->num_inputs; j++)
  //aiger_add_input (dst, src->inputs[j].lit, src->inputs[j].name);

  //for (j = 0; j < src->num_ands; j++)
  //{
  //a = src->ands + j;
  //aiger_add_and (dst, a->lhs, a->rhs0, a->rhs1);
  //}

  //if (src->num_outputs)
  //{
  //out = src->outputs[0].lit;
  //for (j = 1; j < src->num_outputs; j++)
  //{
  //tmp = 2 * (dst->maxvar + 1);
  //aiger_add_and (dst, tmp, out, src->outputs[j].lit);
  //out = tmp;
  //}
  //aiger_add_output (dst, out, "AIGER_AND");
  //}

  aiger_reset (src);

  return ret;
}
