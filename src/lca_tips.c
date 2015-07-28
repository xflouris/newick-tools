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

void lca_tips(rtree_t * root, rtree_t ** tip1, rtree_t ** tip2)
{
  if (!root->left)
  {
    *tip1 = root;
    *tip2 = NULL;
    return;
  }

  *tip1 = root->left;
  *tip2 = root->left;

  while ((*tip1)->left)
    *tip1 = (*tip1)->left;

  while ((*tip2)->right)
    *tip2 = (*tip2)->right;

  return;
}
