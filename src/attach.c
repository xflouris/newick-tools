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

void cmd_attach_tree(void)
{
  FILE * out;
  unsigned int i;
  /* make sure to do all conversions */ 

  
  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
    fatal("Currently only rooted trees are supported");

  rtree_t * attachtree = rtree_parse_newick(opt_attach_filename);
  if (!attachtree)
    fatal("Currently only rooted trees are supported");


  rtree_t ** tip_nodes = (rtree_t **)xmalloc(rtree->leaves *
                                             sizeof(rtree_t *));

  rtree_query_tipnodes(rtree, tip_nodes);

  for (i=0; i < rtree->leaves; ++i)
    if (!strcmp(tip_nodes[i]->label,opt_attach_at))
      break;

  if (i == rtree->leaves)
    fatal("Attach at tip not found");
  
  free(tip_nodes[i]->label);
  tip_nodes[i]->label = NULL;

  if (tip_nodes[i]->parent->left == tip_nodes[i])
    tip_nodes[i]->parent->left = attachtree;
  else
    tip_nodes[i]->parent->right = attachtree;

  attachtree->parent  = tip_nodes[i]->parent;
  attachtree->length += tip_nodes[i]->length;

  rtree_destroy(tip_nodes[i]);

  free(tip_nodes);

  
  rtree_reset_leaves(rtree);

  /* prepare for output */
  char * newick = rtree_export_newick(rtree);

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  
  fprintf(out, "%s\n", newick); 

  if (opt_outfile)
    fclose(out);

  rtree_destroy(rtree);
  free(newick);
  
}
