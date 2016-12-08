/*
    Copyright (C) 2015 Tomas Flouri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact: Tomas Flouri <Tomas.Flouri@h-its.org>,
    Heidelberg Institute for Theoretical Studies,
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

#include "newick-tools.h"

static double roundfactor;

static double fround(double x)
{
  return round(x*roundfactor)/roundfactor;
}

static int cb_asc(const void * va, const void * vb)
{
  double a = *(double *)va;
  double b = *(double *)vb;

  if (a - b > 0) return 1;
  else if (a - b < 0) return -1;

  return 0;
}

static double create_waiting_time(double s, double t)
{
  if (opt_birthrate == opt_deathrate)
  {
    return (s*t) / (1 + opt_birthrate*t*(1-s));
  }

  double diff = opt_deathrate - opt_birthrate;
  double e = exp(diff*t);

  double num = opt_birthrate - opt_deathrate*e - opt_deathrate*(1 - e)*s;
  double denum = opt_birthrate - opt_deathrate*e - opt_birthrate*(1 - e)*s;

  double terma = 1 / (opt_birthrate - opt_deathrate);
  double termb = log(num/denum);

  return terma*termb;
}

static double origin(double t)
{
  double terma;
  double num;
  double denum;
  double termb;

  /* birth rate = death rate */
  if (opt_birthrate == opt_deathrate)
  {
    num = 1.0;
    denum = opt_birthrate * (pow(t,-1.0/opt_simulate_tips) - 1);
    return num/denum;
  }

  /* otherwise */
  terma = 1 / (opt_birthrate - opt_deathrate);
  num   = 1 - (opt_deathrate/opt_birthrate)*pow(t,1.0/opt_simulate_tips);
  denum = 1 - pow(t,1.0/opt_simulate_tips);
  termb = log(num/denum);

  return terma*termb;
}

static void set_branchlength(rtree_t * node, double parent_age)
{
  double * node_ageptr;

  if (!node->data)
    node->length = parent_age;
  else
  {
    node_ageptr = (double *)(node->data);
    node->length = parent_age - *node_ageptr;
  }
}

void cmd_simulate_bd(void)
{
  FILE * out;
  int i;
  char ** labels = NULL;

  roundfactor = pow(10, opt_precision);

  /* create an array of nodes */

  rtree_t ** children = (rtree_t **)xmalloc(opt_simulate_tips*sizeof(rtree_t *));

  if (opt_labels)
  {
    int count;
    labels = parse_labels(opt_labels, &count); 
    if (count != opt_simulate_tips)
      fatal("Number of labels in %s differs from --opt_simulate_bd",
            opt_labels);

  }

  for (i=0; i<opt_simulate_tips; ++i)
  {
    children[i] = (rtree_t *)xcalloc(1,sizeof(rtree_t));
    children[i]->leaves = 1;
    if (labels)
      children[i]->label = labels[i];
    else
      asprintf(&(children[i]->label), "%d", i+1);
  }

  if (labels)
    free(labels);

  /* compute origin in absolute */
  double t = origin(rnd_uniform(0,1));

  /* allocate space for tips-1 waiting times */
  double * s = (double *)xmalloc((opt_simulate_tips-1)*sizeof(double));

  
  /* compute waiting times */
  for (i=0; i<opt_simulate_tips-1; ++i)
    s[i] = create_waiting_time(rnd_uniform(0,1), t);

  /* re-scale if specified */
  if (opt_origin_scale)
  {
    double scaler = opt_origin / t; 

    t = opt_origin;

    for (i=0; i<opt_simulate_tips-1; ++i)
      s[i] *= scaler;
  }

  /* round to the decimal point specified by opt_precision */
  t = fround(t);
  for (i=0; i<opt_simulate_tips-1; ++i)
    s[i] = fround(s[i]);

  /* sort them from smallest to largest */
  qsort((void *)s, opt_simulate_tips-1, sizeof(double), cb_asc);


  rtree_t * new;
  /* randomly resolve current node */
  i = opt_simulate_tips;
  while (i != 1)
  {
    /* select two children such that r1 < r2 */
    int r1 = rand() % i;
    int r2 = rand() % i;
    if (r1 == r2)
      r2 = (r1 == i-1) ? r2-1 : r2+1;
    if (r1 > r2) SWAP(r1,r2);

    /* create a new inner node */
    new = (rtree_t *)xmalloc(sizeof(rtree_t));
    new->left   = children[r1];
    new->right  = children[r2];
    new->leaves = new->left->leaves + new->right->leaves;
    new->length = 0;
    new->label  = NULL;
    new->mark   = 0;
    new->color  = NULL;

    /* store pointer to waiting time */
    new->data   = (void *)(s + opt_simulate_tips - i);

    set_branchlength(new->left,  s[opt_simulate_tips-i]);
    set_branchlength(new->right, s[opt_simulate_tips-i]);

    new->left->data  = NULL;
    new->right->data = NULL;

    new->left->parent = new;
    new->right->parent = new;

    /* update list of children with new inner node and remove old
       invalid children */
    children[r1] = new;
    if (r2 != i-1)
      children[r2] = children[i-1];

    --i;
  }

  /* new is the root */
  set_branchlength(new, t);
  new->data = NULL;
  new->parent = NULL;

//  /* scale time of origin */
//  if (opt_origin_scale)
//  {
//    double scaler = opt_origin / t; 
//
//    rtree_t ** nodes = (rtree_t **)xmalloc((2*opt_simulate_tips-1) *
//                                           sizeof(rtree_t *));
//
//    rtree_query_tipnodes(new, nodes);
//    rtree_query_innernodes(new, nodes+opt_simulate_tips);
//
//    for (i=0; i <2*opt_simulate_tips-1; ++i)
//      nodes[i]->length *= scaler;
//
//    double maxlength = rtree_longest_path(new);
//
//    /* correct any numerical errors */
//    new->length += opt_origin - maxlength;
//    free(nodes);
//    assert(new->length > 0);
//  }

  /* prepare for output */
  char * newick = rtree_export_newick(new);

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  
  fprintf(out, "%s\n", newick); 

  if (opt_outfile)
    fclose(out);

  rtree_destroy(new);
  free(children);
  free(s);
  free(newick);
}
