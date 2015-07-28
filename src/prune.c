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
      temp->parent = grandparent;
    }
    else
    {
      temp->parent = NULL;
      temp->length = 0;
      *root = temp;
    }
    free(parent);
  }
}

void cmd_prune_tips()
{
  FILE * out;
  rtree_t ** prune_tips_list;
  unsigned int prune_tips_count;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  prune_tips_list = rtree_tipstring_nodes(rtree,
                                          opt_prune_tips,
                                          &prune_tips_count);

  prune_taxa(&rtree, prune_tips_list, prune_tips_count);
  free(prune_tips_list);

  rtree_reset_leaves(rtree);

  /* export tree structure to newick string */
  char * newick = rtree_export_newick(rtree);

  fprintf(out, "%s\n", newick);

  if (opt_outfile)
    fclose(out);

  free(newick);

  /* deallocate tree structure */
  rtree_destroy(rtree);

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
