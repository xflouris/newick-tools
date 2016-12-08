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

static void prune_taxa(rtree_t ** root,
                       rtree_t ** prune_tips_list,
                       unsigned int prune_tips_count)
{
  unsigned int i;

  if (prune_tips_count > (*root)->leaves-2)
    fatal("Error, the resulting tree must have at least 2 taxa.");
    
  for (i = 0; i < prune_tips_count; ++i)
  {
    rtree_t * parent = prune_tips_list[i]->parent;
    rtree_t * grandparent = parent->parent;

    if (!grandparent)
    {
      fprintf(stderr,
            "Warning: All taxa from one root subtree deleted. Root changed.\n");
    }
    
    rtree_t * temp = (parent->left == prune_tips_list[i]) ?
                           parent->right : parent->left;
    
    if (prune_tips_list[i]->label)
      free(prune_tips_list[i]->label);
    free(prune_tips_list[i]);

    if (parent->label)
      free(parent->label);

    if (grandparent)
    {
      if (grandparent->left == parent)
      {
        grandparent->left = temp;
      }
      else
      {
        grandparent->right = temp;
      }
      temp->length += parent->length;
      temp->parent = grandparent;
    }
    else
    {
      temp->parent = NULL;
      /* the next line will zero-out the root branch (origin) */
      /* temp->length = 0; */
      *root = temp;
    }
    free(parent);
  }
}

static utree_t * utree_prune_taxa(utree_t ** prune_tips_list,
                                  unsigned int prune_tips_count)
{
  unsigned int i;
  /* TODO: Check if we can remove that many tips */
  utree_t * root = NULL;

  for (i = 0; i < prune_tips_count; ++i)
  {
    utree_t * parent = prune_tips_list[i]->back;
    
    utree_t * x = parent->next->back;
    utree_t * y = parent->next->next->back;

    double len = x->length + y->length;

    free(parent->next->next);
    free(parent->next);
    free(parent->label);
    free(parent);

    x->back = y;
    y->back = x;
    x->length = y->length = len;

    free(prune_tips_list[i]->label);
    free(prune_tips_list[i]);

    if (!(x->next))
      root = y;
    else
      root = x;
  }

  return root;
}
static utree_t ** utree_random_tiplist(utree_t * root, int tip_count)
{
  utree_t ** node_list = (utree_t **)xmalloc(tip_count * sizeof(utree_t *));
  utree_query_tipnodes(root, node_list);
  shuffle((void *)node_list, tip_count, sizeof(utree_t *));
  return node_list;
}

static rtree_t ** rtree_random_tiplist(rtree_t * root)
{
  rtree_t ** node_list = (rtree_t **)xmalloc(root->leaves * sizeof(rtree_t *));
  rtree_query_tipnodes(root, node_list);
  shuffle((void *)node_list, root->leaves, sizeof(rtree_t *));
  return node_list;
}
                        
void cmd_prune_tips()
{
  FILE * out;
  unsigned int prune_tips_count;
  char * newick;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
  {
    int tip_count;
    utree_t * utree = utree_parse_newick(opt_treefile, &tip_count);
    if (!utree)
      fatal("Tree is neither unrooted nor rooted. Go fix your tree.");

    if (!opt_quiet)
      fprintf(stdout, "Loaded unrooted tree...\n");

    utree_t ** prune_tips_list;

    if (opt_prune_random)
    {
      prune_tips_list = utree_random_tiplist(utree, tip_count);
      prune_tips_count = opt_prune_random;
    }
    else
      prune_tips_list = utree_tipstring_nodes(utree,
                                              tip_count,
                                              opt_prune_tips,
                                              &prune_tips_count);
      
    if (prune_tips_count+3 > (unsigned int)tip_count)
      fatal("Error, the resulting tree must have at least 3 taxa.");

    utree_t * uroot = utree_prune_taxa(prune_tips_list, prune_tips_count);
    free(prune_tips_list);

    /* export tree structure to newick string */
    newick = utree_export_newick(uroot);

    /* deallocate tree structure */
    utree_destroy(uroot);
  }
  else
  {
    rtree_t ** prune_tips_list;
   
    if (opt_prune_random)
    {
      prune_tips_list = rtree_random_tiplist(rtree);
      prune_tips_count = opt_prune_random;
    }
    else
      prune_tips_list = rtree_tipstring_nodes(rtree,
                                              opt_prune_tips,
                                              &prune_tips_count);

    prune_taxa(&rtree, prune_tips_list, prune_tips_count);
    free(prune_tips_list);

    rtree_reset_leaves(rtree);

    /* export tree structure to newick string */
    newick = rtree_export_newick(rtree);

    /* deallocate tree structure */
    rtree_destroy(rtree);
  }



  fprintf(out, "%s\n", newick);

  if (opt_outfile)
    fclose(out);

  free(newick);

  if (!opt_quiet)
    fprintf(stdout, "Done...\n");
}

void cmd_induce_tree()
{
  FILE * out;
  rtree_t ** complement_tiplist;
  rtree_t ** prune_tiplist;
  unsigned int complement_tips_count = 0;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  complement_tiplist = rtree_tipstring_nodes(rtree,
                                           opt_induce_subtree,
                                           &complement_tips_count);

  prune_tiplist = rtree_tiplist_complement(rtree,
                                           complement_tiplist,
                                           complement_tips_count);
  free(complement_tiplist);

  prune_taxa(&rtree, prune_tiplist, rtree->leaves - complement_tips_count);
  free(prune_tiplist);
   
  rtree_reset_leaves(rtree);
  char * newick = rtree_export_newick(rtree);

  fprintf(out, "%s\n", newick);

  free(newick);

  /* deallocate tree structure */
  rtree_destroy(rtree);

  if (!opt_quiet)
    fprintf(stdout, "Done...\n");

  if (opt_outfile)
    fclose(out);
}
