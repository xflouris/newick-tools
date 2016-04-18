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

%{
#include "newick-tools.h"

extern int ntree_lex();
extern FILE * ntree_in;
extern void ntree_lex_destroy();

struct forest_s
{
  ntree_t ** children;
  int count;
};

void ntree_destroy(ntree_t * root)
{
  int i;

  if (!root) return;

  if (root->children)
  {
    for (i = 0; i < root->children_count; ++i)
    {
      ntree_destroy(root->children[i]);
    }
    free(root->children);
  }
  
  free(root->label);
  free(root);
}


static void ntree_error(ntree_t * tree, const char * s) 
{
}

%}


%union
{
  char * s;
  char * d;
  struct ntree_s * tree;
  struct forest_s * forest;
}

%error-verbose
%parse-param {struct ntree_s * tree}
%destructor { ntree_destroy($$); } subtree

%token OPAR
%token CPAR
%token COMMA
%token COLON SEMICOLON 
%token<s> STRING
%token<d> NUMBER
%type<s> label optional_label
%type<d> number optional_length
%type<tree> subtree
%type<forest> forest
%start input
%%

input: subtree SEMICOLON
{
  memcpy(tree,$1,sizeof(ntree_t));
  tree->parent = NULL;
  free($1);
};

forest: forest COMMA subtree
{
  /* allocate space for one more subtree */
  ntree_t ** children = (ntree_t **)calloc($1->count + 1, sizeof(ntree_t *));
  memcpy(children, $1->children, $1->count * sizeof(ntree_t *));
  children[$1->count] = $3;
  free($1->children);
  $1->children = children;
  $1->count++;

  $$ = $1;
}
      | subtree
{
  $$ = (struct forest_s *)calloc(1, sizeof(struct forest_s));
  $$->children = (ntree_t **)calloc(1,sizeof(ntree_t *));
  $$->children[0] = $1;
  $$->count = 1;
}

subtree: OPAR forest CPAR optional_label optional_length
{
  int i;

  $$ = (ntree_t *)calloc(1, sizeof(ntree_t));
  $$->children = $2->children;
  $$->label = $4;
  $$->length = $5 ? atof($5) : 0;
  $$->children_count = $2->count;

  for (i = 0; i < $2->count; ++i)
    $$->children[i]->parent = $$;
  $$->mark = 0;

  free($2);
  free($5);
}
       | label optional_length
{
  $$ = (ntree_t *)calloc(1, sizeof(ntree_t));
  $$->label  = $1;
  $$->length = $2 ? atof($2) : 0;
  $$->children = NULL;
  $$->children_count = 0;
  $$->mark   = 0;
  free($2);
};

 
optional_label:  {$$ = NULL;} | label  {$$ = $1;};
optional_length: {$$ = NULL;} | COLON number {$$ = $2;};
label: STRING    {$$=$1;} | NUMBER {$$=$1;};
number: NUMBER   {$$=$1;};

%%

ntree_t * ntree_parse_newick(const char * filename)
{
  struct ntree_s * tree;

  tree = (ntree_t *)calloc(1, sizeof(ntree_t));

  ntree_in = fopen(filename, "r");
  if (!ntree_in)
  {
    ntree_destroy(tree);
    snprintf(errmsg, 200, "Unable to open file (%s)", filename);
    return NULL;
  }
  else if (ntree_parse(tree))
  {
    ntree_destroy(tree);
    tree = NULL;
    fclose(ntree_in);
    ntree_lex_destroy();
    return NULL;
  }
  
  if (ntree_in) fclose(ntree_in);

  ntree_lex_destroy();

  return tree;
}
