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

int ntree_tipcount(ntree_t * node)
{
  int i;
  int count = 0;

  if (!node) return 0;
  if (!node->children_count == 0) return 1;

  for (i=0; i<node->children_count; ++i)
    count += ntree_tipcount(node->children[i]);

  return count;
}

static void ntree_query_tipnodes_recursive(ntree_t * node,
                                           ntree_t ** node_list,
                                           int * index) 
{
  int i;
  if (!node) return;

  if (!node->children_count)
  {
    node_list[*index] = node;
    *index = *index + 1;
    return;
  }

  for (i=0; i<node->children_count; ++i)
    ntree_query_tipnodes_recursive(node->children[i], node_list, index);
}


ntree_t ** ntree_query_tipnodes(ntree_t * node, int * count)
{
  int i;
  ntree_t ** node_list;

  if (!node) return 0;

  *count = 0;

  /* the passed node is a tip */
  if (!node->children_count)
  {
    node_list = (ntree_t **)xmalloc(sizeof(ntree_t *));
    node_list[*count] = node;
    *count = 1;
  }

  /* count number of tips and allocate memory */
  int tipcount = ntree_tipcount(node);
  node_list = (ntree_t **)xmalloc(tipcount*sizeof(ntree_t *));

  /* traverse all subtrees fill the array */
  for (i=0; i<node->children_count; ++i)
    ntree_query_tipnodes_recursive(node->children[i], node_list, count);

  return node_list;
}

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

static rtree_t * resolve_random(ntree_t * node)
{
  int i;
  rtree_t * rtree;
  double length = 0;

  while (node->children_count == 1)
  {
    length += node->length;
    node = node->children[0];
  }
  length += node->length;

  /* allocate node */
  rtree = (rtree_t *)xmalloc(sizeof(rtree_t));
  rtree->mark   = 0;
  rtree->color  = NULL;
  rtree->data   = NULL;
  rtree->label  = (node->label) ? xstrdup(node->label) : NULL;
  rtree->length = length;


  if (node->children_count == 0)
  {
    rtree->left   = NULL;
    rtree->right  = NULL;
    rtree->leaves = 1;
  }
  else if (node->children_count == 2)
  {
    rtree->left   = resolve_random(node->children[0]);
    rtree->right  = resolve_random(node->children[1]);
    rtree->leaves = rtree->left->leaves + rtree->right->leaves;

    rtree->left->parent  = rtree;
    rtree->right->parent = rtree;
  }
  else
  {
    /* allocate array to hold resolved subtrees of the children of node */
    rtree_t ** children = (rtree_t **)xmalloc(node->children_count *
                                              sizeof(rtree_t *));

    /* resolve all children */
    for (i=0; i < node->children_count; ++i)
      children[i] = resolve_random(node->children[i]);

    /* randomly resolve current node */
    i = node->children_count;
    while (i != 2)
    {
      /* select two children such that r1 < r2 */
      int r1 = (rand() % i);
      int r2 = (rand() % i);
      if (r1 == r2)
        r2 = (r1 == i-1) ? r2-1 : r2+1;
      if (r1 > r2) SWAP(r1,r2);

      /* create a new node */
      rtree_t * new = (rtree_t *)xmalloc(sizeof(rtree_t));
      new->left   = children[r1];
      new->right  = children[r2];
      new->leaves = new->left->leaves + new->right->leaves;
      new->length = 0;
      new->label  = NULL;
      new->mark   = 0;
      new->color  = NULL;
      new->data   = NULL;

      new->left->parent = new;
      new->right->parent = new;

      /* update list of children with new inner node and remove old
         invalid children */
      children[r1] = new;
      if (r2 != i-1)
        children[r2] = children[i-1];

      --i;
    }

    /* now we have two children */
    rtree->left   = children[0];
    rtree->right  = children[1];
    rtree->leaves = rtree->left->leaves + rtree->right->leaves;

    rtree->left->parent  = rtree;
    rtree->right->parent = rtree;

    free(children);
  }

  return rtree;

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
    rtree->data = NULL;

    /* caterpillar subtree */
    if (node->children == NULL)
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
    rtree->data = NULL;

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

  /* we want to resolve randomly */
  if (!opt_resolve_ladder)
  {
    rtree = resolve_random(root);
    rtree->parent = NULL;
    return rtree;
  }

  /* otherwise we want to resolve the multifurcations in a ladder way */

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
  rtree->data = NULL;

  return rtree;
}
