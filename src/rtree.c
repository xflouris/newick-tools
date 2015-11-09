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

static int indend_space = 4;

static void print_node_info(FILE * stream, rtree_t * tree)
{
  fprintf(stream," %s", tree->label);
  fprintf(stream," %f", tree->length);
  fprintf(stream,"\n");
}

static void print_tree_recurse(FILE * stream,
                               rtree_t * tree, 
                               int indend_level, 
                               int * active_node_order)
{
  int i,j;

  if (!tree) return;

  for (i = 0; i < indend_level; ++i)
  {
    if (active_node_order[i])
      fprintf(stream,"|");
    else
      fprintf(stream," ");

    for (j = 0; j < indend_space-1; ++j)
      fprintf(stream," ");
  }
  fprintf(stream,"\n");

  for (i = 0; i < indend_level-1; ++i)
  {
    if (active_node_order[i])
      fprintf(stream,"|");
    else
      fprintf(stream," ");

    for (j = 0; j < indend_space-1; ++j)
      fprintf(stream," ");
  }

  fprintf(stream,"+");
  for (j = 0; j < indend_space-1; ++j)
    fprintf (stream,"-");
  if (tree->left || tree->right)
    fprintf(stream,"+");

  print_node_info(stream,tree);

  if (active_node_order[indend_level-1] == 2) 
    active_node_order[indend_level-1] = 0;

  active_node_order[indend_level] = 1;
  print_tree_recurse(stream,
                     tree->left,
                     indend_level+1,
                     active_node_order);
  active_node_order[indend_level] = 2;
  print_tree_recurse(stream,
                     tree->right,
                     indend_level+1,
                     active_node_order);

}

static int tree_indend_level(rtree_t * tree, int indend)
{
  if (!tree) return indend;

  int a = tree_indend_level(tree->left,  indend+1);
  int b = tree_indend_level(tree->right, indend+1);

  return (a > b ? a : b);
}

void rtree_show_ascii(FILE * stream, rtree_t * tree)
{
  
  int indend_max = tree_indend_level(tree,0);

  int * active_node_order = (int *)malloc((indend_max+1) * sizeof(int));
  active_node_order[0] = 1;
  active_node_order[1] = 1;

  print_node_info(stream,tree);
  print_tree_recurse(stream, tree->left,  1, active_node_order);
  active_node_order[0] = 2;
  print_tree_recurse(stream, tree->right, 1, active_node_order);
  free(active_node_order);
}

static char * rtree_export_newick_recursive(rtree_t * root)
{
  char * newick;

  if (!root) return NULL;

  if (!(root->left) || !(root->right))
    asprintf(&newick, "%s:%f", root->label, root->length);
  else
  {
    char * subtree1 = rtree_export_newick_recursive(root->left);
    char * subtree2 = rtree_export_newick_recursive(root->right);

    asprintf(&newick, "(%s,%s)%s:%f", subtree1,
                                      subtree2,
                                      root->label ? root->label : "",
                                      root->length);
    free(subtree1);
    free(subtree2);
  }

  return newick;
}

char * rtree_export_newick(rtree_t * root)
{
  char * newick;

  if (!root) return NULL;

  if (!(root->left) || !(root->right))
    asprintf(&newick, "%s:%f", root->label, root->length);
  else
  {
    char * subtree1 = rtree_export_newick_recursive(root->left);
    char * subtree2 = rtree_export_newick_recursive(root->right);

    asprintf(&newick, "(%s,%s)%s:%f;", subtree1,
                                       subtree2,
                                       root->label ? root->label : "",
                                       root->length);
    free(subtree1);
    free(subtree2);
  }

  return newick;
}

static void rtree_traverse_recursive(rtree_t * node,
                                     int (*cbtrav)(rtree_t *),
                                     int * index,
                                     rtree_t ** outbuffer)
{
  if (!node->left)
  {
    if (cbtrav(node))
    {
      outbuffer[*index] = node;
      *index = *index + 1;
    }
    return;
  }
  if (!cbtrav(node))
    return;
  rtree_traverse_recursive(node->left, cbtrav, index, outbuffer);
  rtree_traverse_recursive(node->right, cbtrav, index, outbuffer);

  outbuffer[*index] = node;
  *index = *index + 1;
}

int rtree_traverse(rtree_t * root,
                   int (*cbtrav)(rtree_t *),
                   rtree_t ** outbuffer)
{
  int index = 0;

  if (!root->left) return -1;

  /* we will traverse an unrooted tree in the following way
      
           root
            /\
           /  \
        left   right

     at each node the callback function is called to decide whether we
     are going to traversing the subtree rooted at the specific node */

  rtree_traverse_recursive(root, cbtrav, &index, outbuffer);
  return index;
}

static void rtree_traverse_postorder_recursive(rtree_t * node,
                                               int (*cbtrav)(rtree_t *),
                                               int * index,
                                               rtree_t ** outbuffer)
{
  if (!node) return;

  rtree_traverse_postorder_recursive(node->left,  cbtrav, index, outbuffer);
  rtree_traverse_postorder_recursive(node->right, cbtrav, index, outbuffer);

  if (cbtrav(node))
  {
    outbuffer[*index] = node;
    *index = *index + 1;
  }
}


int rtree_traverse_postorder(rtree_t * root,
                             int (*cbtrav)(rtree_t *),
                             rtree_t ** outbuffer)
{
  int index = 0;

  if (!root->left) return -1;

  /* we will traverse an unrooted tree in the following way
      
           root
            /\
           /  \
        left   right

     at each node the callback function is called to decide whether to
     place the node in the list */

  rtree_traverse_postorder_recursive(root, cbtrav, &index, outbuffer);
  if (cbtrav(root))
  {
    outbuffer[index] = root;
    index = index + 1;
  }
  return index;
}

void rtree_traverse_sorted(rtree_t * root, rtree_t ** node_list, int * index)
{
  if (!root) return;
  if (!root->left)
  {
    node_list[(*index)++] = root;
    return;
  }

  int left_start = *index;
  rtree_traverse_sorted(root->left, node_list, index);
  int right_start = *index;
  rtree_traverse_sorted(root->right, node_list, index);

  int left_len = right_start - left_start;
  int right_len = *index - right_start;

  int swap = 0;

  /* check whether to swap subtrees based on subtree size */
  if (right_len < left_len)
  {
    swap = 1;
  }
  else if (right_len == left_len)
  {
    int i;

    for (i = 0; i < left_len; ++i)
    {
      rtree_t * left_node = node_list[left_start+i];
      rtree_t * right_node = node_list[right_start+i];

      /* check whether to swap subtrees based on first tip occurrence */
      if (left_node->left == NULL && right_node->left != NULL)
      {
        swap = 0;
        break;
      }
      else if (left_node->left != NULL && right_node->left == NULL)
      {
        swap = 1;
        break;
      }

      /* check whether to swap based on smaller label */
      int cmp = strcmp(left_node->label, right_node->label);
      if (cmp < 0)
      {
        swap = 0;
        break;
      }
      else if (cmp > 0)
      {
        swap = 1;
        break;
      }
    }

  }

  if (swap)
  {
    /* swap the two trees */
    rtree_t ** temp = (rtree_t **)malloc(left_len * sizeof(rtree_t *));
    memcpy(temp, node_list+left_start, left_len * sizeof(rtree_t *));
    memcpy(node_list+left_start, node_list+right_start, right_len * sizeof(rtree_t *));
    memcpy(node_list+left_start+right_len, temp, left_len * sizeof(rtree_t *));
    free(temp);
  }

  node_list[(*index)++] = root;
}

static void rtree_query_tipnodes_recursive(rtree_t * node,
                                           rtree_t ** node_list,
                                           int * index)
{
  if (!node) return;

  if (!node->left)
  {
    node_list[*index] = node;
    *index = *index + 1;
    return;
  }

  rtree_query_tipnodes_recursive(node->left,  node_list, index);
  rtree_query_tipnodes_recursive(node->right, node_list, index);
}

int rtree_query_tipnodes(rtree_t * root,
                         rtree_t ** node_list)
{
  int index = 0;

  if (!root) return 0;
  if (!root->left)
  {
    node_list[index++] = root;
    return index;
  }

  rtree_query_tipnodes_recursive(root->left,  node_list, &index);
  rtree_query_tipnodes_recursive(root->right, node_list, &index);

  return index;
}

static void rtree_query_innernodes_recursive(rtree_t * root,
                                             rtree_t ** node_list,
                                             int * index)
{
  if (!root) return;
  if (!root->left) return;

  /* postorder traversal */

  rtree_query_innernodes_recursive(root->left,  node_list, index);
  rtree_query_innernodes_recursive(root->right, node_list, index);

  node_list[*index] = root;
  *index = *index + 1;
  return;
}

int rtree_query_innernodes(rtree_t * root,
                           rtree_t ** node_list)
{
  int index = 0;

  if (!root) return 0;
  if (!root->left) return 0;

  rtree_query_innernodes_recursive(root->left,  node_list, &index);
  rtree_query_innernodes_recursive(root->right, node_list, &index);

  node_list[index++] = root;

  return index;
}

void rtree_reset_leaves(rtree_t * root)
{
  if (!root->left)
  {
    root->leaves = 1;
    return;
  }
  
  rtree_reset_leaves(root->left);
  rtree_reset_leaves(root->right);

  root->leaves = root->left->leaves + root->right->leaves;
}

rtree_t ** rtree_tipstring_nodes(rtree_t * root, char * tipstring, unsigned int * tiplist_count)
{
  unsigned int i;
  unsigned int k;
  unsigned int commas_count = 0;

  char * taxon;
  unsigned int taxon_len;

  ENTRY * found = NULL;

  for (i = 0; i < strlen(tipstring); ++i)
    if (tipstring[i] == ',')
      commas_count++;
  
  rtree_t ** node_list = (rtree_t **)xmalloc(root->leaves * sizeof(rtree_t *));
  rtree_query_tipnodes(root, node_list);

  rtree_t ** out_node_list = (rtree_t **)xmalloc((commas_count+1) *
                                                   sizeof(rtree_t *));

  /* create a hashtable of tip labels */
  hcreate(2 * root->leaves);

  for (i = 0; i < root->leaves; ++i)
  {
    ENTRY entry;
    entry.key  = node_list[i]->label;
    entry.data = node_list[i];
    hsearch(entry,ENTER);
  }

  char * s = tipstring;
  
  k = 0;
  while (*s)
  {
    /* get next tip */
    taxon_len = strcspn(s, ",");
    if (!taxon_len)
      fatal("Erroneous prune list format (double comma)/taxon missing");

    taxon = strndup(s, taxon_len);

    /* search tip in hash table */
    ENTRY query;
    query.key = taxon;
    found = NULL;
    found = hsearch(query,FIND);
    
    if (!found)
      fatal("Taxon %s in does not appear in the tree", taxon);

    /* store pointer in output list */
    out_node_list[k++] = (rtree_t *)(found->data);

    /* free tip label, and move to the beginning of next tip if available */
    free(taxon);
    s += taxon_len;
    if (*s == ',') 
      s += 1;
  }

  /* kill the hash table */
  hdestroy();

  free(node_list);

  /* return number of tips in the list */
  *tiplist_count = commas_count + 1;

  /* return tip node list */
  return out_node_list;
}

rtree_t ** rtree_tiplist_complement(rtree_t * root,
                                    rtree_t ** tiplist,
                                    unsigned int tiplist_count)
{
  unsigned int i;
  unsigned int k;
  ENTRY * found = NULL;

  hcreate(2 * tiplist_count);

  for (i = 0; i < tiplist_count; ++i)
  {
    ENTRY entry;
    entry.key  = tiplist[i]->label;
    entry.data = tiplist[i];
    hsearch(entry,ENTER);
  }
  
  rtree_t ** node_list = (rtree_t **)xmalloc(root->leaves * sizeof(rtree_t *));
  rtree_query_tipnodes(root, node_list);

  rtree_t ** out_node_list = (rtree_t **)xmalloc((root->leaves - tiplist_count)*
                                                 sizeof(rtree_t *));
  
  for (k = 0, i = 0; i < root->leaves; ++i)
  {
    ENTRY query;
    query.key = node_list[i]->label;
    found = NULL;
    found = hsearch(query,FIND);
    
    /* store pointer in output list */
    if (!found)
      out_node_list[k++] = node_list[i];
  }

  hdestroy();
  free(node_list);

  assert(k == (root->leaves - tiplist_count));

  return out_node_list;
}
