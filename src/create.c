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

void cmd_randomtree_binary(void)
{
  FILE * out;
  int i;
  char * label;

  /* create array of nodes */
  rtree_t ** nodes = (rtree_t **)xmalloc(opt_randomtree_tips *
                                         sizeof(rtree_t *));

  /* allocate tip nodes */
  for (i = 0; i < opt_randomtree_tips; ++i)
  {
    nodes[i] = (rtree_t *)xmalloc(sizeof(rtree_t));
    asprintf(&label, "%d", i);
    nodes[i]->label = label;
    nodes[i]->leaves = 1;
    nodes[i]->left = nodes[i]->right = NULL;
    nodes[i]->length = rnd_uniform(opt_randomtree_minbranch,
                                   opt_randomtree_maxbranch);
    nodes[i]->data = NULL;
  }

  int count = opt_randomtree_tips;

  while (count != 1)
  {
    /* randomly select first node */
    i = random() % count;
    rtree_t * a = nodes[i];

    /* in case we did not select the last node in the list, move the last
       node to the position of the node we selected */
    if (i != count-1)
      nodes[i] = nodes[count-1];

    /* decrease number of nodes in list */
    --count;

    /* randomly select second node */
    i = random() % count;
    rtree_t * b = nodes[i];

    /* in case we did not select the last node in the list, move the last
       node to the position of the node we selected */
    if (i != count-1)
      nodes[i] = nodes[count-1];

    /* decrease number of nodes in list */
    --count;

    nodes[count] = (rtree_t *)xmalloc(sizeof(rtree_t));
    nodes[count]->parent = NULL;
    nodes[count]->left = a;
    nodes[count]->right = b;
    nodes[count]->left->parent = nodes[count];
    nodes[count]->right->parent = nodes[count];
    nodes[count]->leaves = a->leaves + b->leaves;
    nodes[count]->data = NULL;

    ++count;

    //asprintf(&label, "%ld", 2*opt_randomtree_tips - count - 1); 
    nodes[count-1]->label = NULL;
    nodes[count-1]->length = rnd_uniform(opt_randomtree_minbranch,
                                         opt_randomtree_maxbranch);
  }

  char * newick = rtree_export_newick(nodes[0]);

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  
  fprintf(out, "%s\n", newick); 

  if (opt_outfile)
    fclose(out);

  rtree_destroy(nodes[0]);
  free(nodes);
  free(newick);

}

#if 0
void cmd_randomtree_nary(void)
{
  FILE * out;
  int i;
  char * label;

  /* TODO: Obtain this from arguments */
  int maxdegree = 5;
  int mindegree = 2;

  if (maxdegree > opt_randomtree_tips)
    fatal("maxdegree must not be larger than tips"); 
  if (mindegree 

  /* create array of nodes */
  rtree_t ** nodes = (rtree_t **)xmalloc(opt_randomtree_tips *
                                         sizeof(rtree_t *));

  /* allocate tip nodes */
  for (i = 0; i < opt_randomtree_tips; ++i)
  {
    int degree = rnd_uniform(
    nodes[i] = (rtree_t *)xmalloc(sizeof(rtree_t));
    asprintf(&label, "%d", i);
    nodes[i]->label = label;
    nodes[i]->leaves = 1;
    nodes[i]->left = nodes[i]->right = NULL;
    nodes[i]->length = rnd_uniform(opt_randomtree_minbranch,
                                   opt_randomtree_maxbranch);
  }

  int count = opt_randomtree_tips;

  while (count != 1)
  {
    
  }

}
#endif
