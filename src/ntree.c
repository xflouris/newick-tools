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

static void ntree_node_count_recursive(ntree_t * root,
                                       int * inner_count,
                                       int * tip_count,
                                       int * min_inner_degree,
                                       int * max_inner_degree)
{
  int i;

  if (!root->children_count)
  {
    *tip_count = *tip_count + 1;
    return;
  }

  *inner_count = *inner_count + 1;
  
  if (root->children_count + 1 > *max_inner_degree)
    *max_inner_degree = root->children_count+1;

  if (root->children_count + 1 < *min_inner_degree)
    *min_inner_degree = root->children_count + 1;

  for (i = 0; i < root->children_count; ++i)
  {
    ntree_node_count_recursive(root->children[i],
                               inner_count,
                               tip_count,
                               min_inner_degree,
                               max_inner_degree);
  }
}

void ntree_node_count(ntree_t * root,
                      int * inner_count,
                      int * tip_count,
                      int * min_inner_degree,
                      int * max_inner_degree)
{
  
  int i;

  *inner_count = 0;
  *tip_count = 0;
  *min_inner_degree = 0;
  *max_inner_degree = 0;

  if (!root->children_count)
  {
    *tip_count = 1;
    return;
  }
  else
    *inner_count = 1;

  *min_inner_degree = root->children_count;
  *max_inner_degree = root->children_count;

  for (i = 0; i < root->children_count; ++i)
  {
    ntree_node_count_recursive(root->children[i],
                               inner_count,
                               tip_count,
                               min_inner_degree,
                               max_inner_degree);
  }
}

static rtree_t * ntree_to_rtree_recursive(ntree_t ** nodes, int count)
{
  rtree_t * rtree;

  ntree_t * node = nodes[0];

  if (count == 1)
  {
    double length = 0;
    while (node->children_count == 1)
    {
      length += node->length;
      node = node->children[0];
    }
    
    rtree = (rtree_t *)xmalloc(sizeof(rtree_t));
    
    rtree->label = (node->label) ? xstrdup(node->label) : NULL;
    rtree->length = node->length + length;
    rtree->color = NULL;
    rtree->mark = 0;

    /* caterpillar subtree */
    if (node->children == 0)
    {
      rtree->left = NULL;
      rtree->right = NULL;
      rtree->leaves = 1;
    }
    else
    {
      /*  n-ary subtree (n > 1) */
      rtree->left = ntree_to_rtree_recursive(node->children, 1);
      rtree->right = ntree_to_rtree_recursive(node->children+1, node->children_count - 1);

      rtree->leaves = rtree->left->leaves + rtree->right->leaves;
      rtree->left->parent = rtree;
      rtree->right->parent = rtree;
    }
  }
  else
  {
    rtree = (rtree_t *)xmalloc(sizeof(rtree_t));
    rtree->length = 0;
    rtree->label = NULL;
    rtree->mark = 0;
    rtree->color = NULL;

    rtree->left = ntree_to_rtree_recursive(nodes,1);
    rtree->right = ntree_to_rtree_recursive(nodes+1,count-1);
    rtree->leaves = rtree->left->leaves + rtree->right->leaves;
    rtree->left->parent = rtree;
    rtree->right->parent = rtree;
  }

  return rtree;
}

rtree_t * ntree_to_rtree(ntree_t * root)
{
  rtree_t * rtree;

  while (root->children_count == 1)
    root = root->children[0];

  if (!root->children_count)
    fatal("Loaded n-tree is a caterpillar tree");

  rtree = (rtree_t *)xmalloc(sizeof(rtree_t));

  /* create left subtrees (postorder traversal) */
  rtree->left = ntree_to_rtree_recursive(root->children, 1);

  /* create right subtrees (postorder traversal) */
  rtree->right = ntree_to_rtree_recursive(root->children+1, root->children_count - 1);

  rtree->parent = NULL;

  rtree->label = (root->label) ? xstrdup(root->label) : NULL;
  rtree->leaves = rtree->left->leaves  + rtree->right->leaves;
  rtree->length = root->length;

  rtree->color = NULL;
  rtree->mark = 0;

  return rtree;
}
