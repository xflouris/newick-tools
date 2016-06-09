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

static FILE * out;

static utree_t * utree_inner_create()
{
  utree_t * node = (utree_t *)calloc(1,sizeof(utree_t));
  
  node = (utree_t *)calloc(1,sizeof(utree_t));
  node->next = (utree_t *)calloc(1,sizeof(utree_t));
  node->next->next = (utree_t *)calloc(1,sizeof(utree_t));
  node->next->next->next = node;

  return node;
}

static void utree_inner_destroy(utree_t * node)
{
  free(node->next->next);
  free(node->next);
  free(node);
}

static utree_t * utree_tip_create()
{
  utree_t * node = (utree_t *)calloc(1,sizeof(utree_t));
  node->next = NULL;

  return node;
}

static void utree_tip_destroy(utree_t * node)
{
  if (node->label)
  free(node);
}

static void swap_ptr(void ** a, void ** b)
{
  void * temp;
  
  temp = *a;
  *a = *b;
  *b = temp;
}

static void utree_link(utree_t * a, utree_t * b)
{
  /*

    *               *               *                * 
     \             /                 \              /
      *---*   *---*        -->        *---*-----*--*
     /    a   b    \                 /    a     b   \
    *               *               *                *

  */

  a->back = b;
  b->back = a;
}

static void utree_edgesplit(utree_t * a, utree_t * b, utree_t * c)
{
  /*
                *                                      *
                |                                      |
                *                                      *
               / \                                    / \
            b *   * c                              b *   * c
                                                    /     \
    *                      *      -->      *       /       \      * 
     \                    /                 \     /         \    /
      *---*----------*---*                   *---*           *--*
     /    a          d    \                 /    a           d   \
    *                      *               *                      *

  */

  /* link d<->c */
  utree_link(a->back,c);

  /* link a<->b */
  utree_link(a,b);
}

static utree_t * root;
static int tree_counter = 0;

static void utree_exhaust_recursive(utree_t ** edge_list, utree_t ** inner_nodes, utree_t ** tip_nodes, int depth)
{
  int i;

  /* find first empty slot in edge_list */
  utree_t ** empty_slot = edge_list;
  while (*empty_slot)
    ++empty_slot;

  /* make the split */
  utree_t * d = (*edge_list)->back;
  utree_edgesplit(*edge_list, *inner_nodes, (*inner_nodes)->next); 
  utree_link((*inner_nodes)->next->next, *tip_nodes);

  /* add the two new edges to the end of the list */
  empty_slot[0] = (*inner_nodes)->next;
  empty_slot[1] = (*inner_nodes)->next->next;

  if (*(tip_nodes+1))
  {
    i = 0;
    while (edge_list[i])
    {
      /* swap the first split with the current */
      if (i)
        swap_ptr((void **)edge_list, (void **)(edge_list+i));

      utree_exhaust_recursive(edge_list, inner_nodes+1,tip_nodes+1,depth+1);

      if (i)
        swap_ptr((void **)edge_list, (void **)(edge_list+i));


      ++i;
    }
  }
  else
  {
    ++tree_counter;
    char * newick =  utree_export_newick(root);
    fprintf(out,"%s\n", newick);
    free(newick);
  }

  /* fall back to the original setting */
  empty_slot[0] = NULL;
  empty_slot[1] = NULL;

  utree_link(*edge_list, d);
  (*inner_nodes)->back = NULL;
  (*inner_nodes)->next->back = NULL;
  (*inner_nodes)->next->next->back = NULL;
  (*tip_nodes)->back = NULL;
}

/* give a list of tip labels as function parameters */
static void utree_exhaust(unsigned int tips_count, char ** tip_labels)
{
  unsigned int i;

  assert(tips_count >= 3);

  /* created the following
          
            *
           /
      *---*
           \
            *
  
  */

  root = utree_inner_create();

  /* create inner node list for (tips_count - 3) inner nodes (root was already
     created, and leave the last slot NULL for termination */
  utree_t ** inner_node_list = (utree_t **)calloc(tips_count - 2, sizeof(utree_t *));

  for (i=0; i<tips_count-3; ++i)
    inner_node_list[i] = utree_inner_create();

  /* create tip node list with a terminating NULL element */
  utree_t ** tip_node_list = (utree_t **)calloc(tips_count+1, sizeof(utree_t *));
  for (i=0; i<tips_count; ++i)
  {
    tip_node_list[i] = utree_tip_create();
    tip_node_list[i]->label = tip_labels[i];
  }

  /* place first three tips */
  utree_link(root, tip_node_list[0]);
  utree_link(root->next, tip_node_list[1]);
  utree_link(root->next->next, tip_node_list[2]);

  /* available placements */
  utree_t ** edge_list = (utree_t **)calloc(2*tips_count-3, sizeof(utree_t *));
  edge_list[0] = root;
  edge_list[1] = root->next;
  edge_list[2] = root->next->next;


  /* call the recursive generator for the first tip for each edge */
  if (tips_count > 3)
  {
    for (i=0; i < 3; ++i)
    {
      /* swap the first split with the current */
      if (i)
        swap_ptr((void **)edge_list, (void **)(edge_list+i));

      utree_exhaust_recursive(edge_list, inner_node_list, tip_node_list+3, 1);

      if (i)
        swap_ptr((void **)edge_list, (void **)(edge_list+i));
    }
  }
  else
  {
    ++tree_counter;
    char * newick =  utree_export_newick(root);
    printf("%s\n", newick);
    free(newick);
  }
  
  /* deallocate inner nodes */
  for (i=0; i < tips_count-3; ++i)
    utree_inner_destroy(inner_node_list[i]);

  /* deallocate tip nodes */
  for (i=0; i < tips_count; ++i)
    utree_tip_destroy(tip_node_list[i]);

  free(inner_node_list);
  free(tip_node_list);

  if (!opt_quiet)
    printf("Total number of topologies: %d\n", tree_counter);
}

void cmd_utree_bf()
{
  FILE * fp;
  const size_t alloc_step = 100;
  size_t alloc_max = 100;
  unsigned int tip_count = 0;

  fp = fopen(opt_alltree_filename,"r");
  if (!fp)
    fatal("Cannot open file %s\n", opt_alltree_filename);

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  char line[4096];

  char ** tip_list;

  tip_list = (char **)xmalloc(alloc_max*sizeof(char*));

  if (!opt_quiet)
    printf("Reading list of tips:\n");
  while (fgets(line,1024,fp))
  {
    if (alloc_max == tip_count)
    {
      alloc_max += alloc_step;
      char ** temp = (char **)xmalloc(alloc_max*sizeof(char*));
      memcpy(temp,tip_list, tip_count*sizeof(char *));
      free(tip_list);
      tip_list=temp;
    }

    long len;
    if (strchr(line,'\r'))
      len = xstrchrnul(line,'\r') - line;
    else
      len = xstrchrnul(line,'\n') - line;

    line[len] = 0;

    tip_list[tip_count] = xstrdup(line);

    if (!opt_quiet)
      printf("%d: %s\n", tip_count+1, line);
    ++tip_count;
  }
  if (!opt_quiet)
    printf("\n");
  
  utree_exhaust(tip_count, tip_list);

  unsigned int i;
  for (i=0; i<tip_count; ++i);
    free(tip_list[i]);
  free(tip_list);
  
  if (opt_outfile)
    fclose(out);
  
  fclose(fp);
}
