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

static int cb_short_trees(rtree_t * node)
{
  /* mark tip down but don't include them in the list */
  if (!node->left)
  {
    node->mark = 1;
    return 0;
  }

  if (node->left->mark && 
      node->right->mark &&
      node->left->length <= opt_subtree_short &&
      node->right->length <= opt_subtree_short)
  {
    node->mark = 1;
    if (node->parent)
    {
      /* if it's parent is the root of a short tree then dont include
         current node in the list, otherwise include it */
      if (node->parent->left->length <= opt_subtree_short &&
          node->parent->right->length <= opt_subtree_short)
      {
        return 0;
      }
      else
      {
        return 1;
      }
    }
    else  /* the current node is the root */
    {
      return 1;
    }
  }

  return 0;

}

void cmd_subtree_short()
{
  FILE * out;
  rtree_t ** inner_node_list;
  rtree_t ** tip_node_list;
  int inner_list_count = 0;
  int tip_list_count = 0;
  int i,j;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);
  if (!rtree)
    fatal("Error: --subtree_short is only implemented for rooted trees");

  fprintf(stdout, "Printing largest subtrees made of branch-lengths smaller "
                  "or equal to %.17f\n", opt_subtree_short);

  /* get inner nodes that are roots of of the largest short subtrees. Short are such subtrees
     where all branch lengths within them are less or equal to opt_subtree_short. The largest
     such subtrees are those that are not subtrees of short subtrees. */
  inner_node_list = (rtree_t **)xmalloc((rtree->leaves-1)*sizeof(rtree_t *));
  inner_list_count = rtree_traverse_postorder(rtree, cb_short_trees, inner_node_list);

  /* traverse the roots and grab the tips */
  tip_node_list = (rtree_t **)xmalloc((rtree->leaves)*sizeof(rtree_t *));
  for (i = 0; i < inner_list_count; ++i)
  {
    tip_list_count = rtree_query_tipnodes(inner_node_list[i], tip_node_list);
    fprintf(stdout, "Subtree %d\n", i+1);
    for (j = 0; j < tip_list_count; ++j)
    {
      fprintf(stdout, "\t%s\n", tip_node_list[j]->label);
    }
  }

  free(inner_node_list);
  free(tip_node_list);

  /* deallocate tree structure */
  rtree_destroy(rtree);

  if (!opt_quiet)
    fprintf(stdout, "Done...\n");
  
  if (opt_outfile)
    fclose(out);

}

