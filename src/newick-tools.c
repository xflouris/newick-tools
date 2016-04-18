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

static char * progname;
static char progheader[80];
static char * cmdline;

/* global error message buffer */
char errmsg[200] = {0};

/* number of mandatory options for the user to input */
static const char mandatory_options_count = 1;
static const char * mandatory_options_list = " --tree_file";

/* options */
char * opt_treefile;
char * opt_outfile;
char * opt_identical;
char * opt_prune_tips;
char * opt_root;
char * opt_induce_subtree;
int opt_quiet;
int opt_precision;
int opt_svg_showlegend;
long opt_help;
long opt_version;
long opt_treeshow;
long opt_lca_left;
long opt_lca_right;
long opt_extract_ltips;
long opt_extract_rtips;
long opt_extract_tips;
long opt_extract_lsubtree;
long opt_extract_rsubtree;
long opt_svg;
long opt_svg_width;
long opt_svg_fontsize;
long opt_svg_tipspace;
long opt_svg_marginleft;
long opt_svg_marginright;
long opt_svg_margintop;
long opt_svg_marginbottom;
long opt_svg_inner_radius;
long opt_prune_random;
long opt_seed;
long opt_info;
long opt_make_binary;
double opt_svg_legend_ratio;
double opt_subtree_short;

static struct option long_options[] =
{
  {"help",               no_argument,       0, 0 },  /*  0 */
  {"version",            no_argument,       0, 0 },  /*  1 */
  {"quiet",              no_argument,       0, 0 },  /*  2 */
  {"tree_file",          required_argument, 0, 0 },  /*  3 */
  {"tree_show",          no_argument,       0, 0 },  /*  4 */
  {"lca_left",           no_argument,       0, 0 },  /*  5 */
  {"lca_right",          no_argument,       0, 0 },  /*  6 */
  {"identical",          required_argument, 0, 0 },  /*  7 */
  {"root",               required_argument, 0, 0 },  /*  8 */
  {"output_file",        required_argument, 0, 0 },  /*  9 */
  {"extract_ltips",      no_argument,       0, 0 },  /* 10 */
  {"extract_rtips",      no_argument,       0, 0 },  /* 11 */
  {"extract_tips",       no_argument,       0, 0 },  /* 12 */
  {"prune_tips",         required_argument, 0, 0 },  /* 13 */
  {"svg",                no_argument,       0, 0 },  /* 14 */
  {"extract_lsubtree",   no_argument,       0, 0 },  /* 15 */
  {"extract_rsubtree",   no_argument,       0, 0 },  /* 16 */
  {"induce_subtree",     required_argument, 0, 0 },  /* 17 */
  {"svg_width",          required_argument, 0, 0 },  /* 18 */
  {"svg_fontsize",       required_argument, 0, 0 },  /* 19 */
  {"svg_tipspacing",     required_argument, 0, 0 },  /* 20 */
  {"svg_legend_ratio",   required_argument, 0, 0 },  /* 21 */
  {"svg_nolegend",       no_argument,       0, 0 },  /* 22 */
  {"svg_marginleft",     required_argument, 0, 0 },  /* 23 */
  {"svg_marginright",    required_argument, 0, 0 },  /* 24 */
  {"svg_margintop",      required_argument, 0, 0 },  /* 25 */
  {"svg_marginbottom",   required_argument, 0, 0 },  /* 26 */
  {"svg_inner_radius",   required_argument, 0, 0 },  /* 27 */
  {"precision",          required_argument, 0, 0 },  /* 28 */
  {"prune_random",       required_argument, 0, 0 },  /* 29 */
  {"seed",               required_argument, 0, 0 },  /* 30 */
  {"subtree_short",      required_argument, 0, 0 },  /* 31 */
  {"info",               no_argument,       0, 0 },  /* 32 */
  {"make_binary",        no_argument,       0, 0 },  /* 33 */
  { 0, 0, 0, 0 }
};

void args_init(int argc, char ** argv)
{
  int option_index = 0;
  int c;
  int mand_options = 0;

  /* set defaults */

  progname = argv[0];

  opt_help = 0;
  opt_version = 0;
  opt_treeshow = 0;
  opt_treefile = NULL;
  opt_lca_left = 0;
  opt_lca_right = 0;
  opt_quiet = 0;
  opt_identical = NULL;
  opt_root = NULL;
  opt_outfile = NULL;
  opt_prune_tips = NULL;
  opt_prune_random = 0;
  opt_extract_ltips = 0;
  opt_extract_rtips = 0;
  opt_extract_tips = 0;
  opt_svg = 0;
  opt_extract_lsubtree = 0;
  opt_extract_rsubtree = 0;
  opt_precision = 7;
  opt_seed = 0;
  opt_svg_width = 1920;
  opt_svg_fontsize = 12;
  opt_svg_tipspace = 20;
  opt_svg_legend_ratio = 0.1;
  opt_svg_showlegend = 1;
  opt_svg_marginleft = 20;
  opt_svg_marginright = 20;
  opt_svg_margintop = 20;
  opt_svg_marginbottom = 20;
  opt_svg_inner_radius = 0;
  opt_subtree_short = -1;
  opt_make_binary = 0;

  while ((c = getopt_long_only(argc, argv, "", long_options, &option_index)) == 0)
  {
    switch (option_index)
    {
      case 0:
        opt_help = 1;
        break;

      case 1:
        opt_version = 1;
        break;

      case 2:
        opt_quiet = 1;
        break;

      case 3:
        free(opt_treefile);
        opt_treefile = optarg;
        break;

      case 4:
        opt_treeshow = 1;
        break;

      case 5:
        opt_lca_left = 1;
        break;

      case 6:
        opt_lca_right = 1;
        break;

      case 7:
        opt_identical = optarg;
        break;

      case 8:
        opt_root = optarg;
        break;

      case 9:
        opt_outfile = optarg;
        break;

      case 10:
        opt_extract_ltips = 1;
        break;

      case 11:
        opt_extract_rtips = 1;
        break;

      case 12:
        opt_extract_tips = 1;
        break;

      case 13:
        opt_prune_tips = optarg;
        break;

      case 14:
        opt_svg = 1;
        break;

      case 15:
        opt_extract_lsubtree = 1;
        break;

      case 16:
        opt_extract_rsubtree = 1;
        break;

      case 17:
        opt_induce_subtree = optarg;
        break;

      case 18:
        opt_svg_width = atoi(optarg);
        break;

      case 19:
        opt_svg_fontsize = atol(optarg);
        break;

      case 20:
        opt_svg_tipspace = atol(optarg);
        break;

      case 21:
        opt_svg_legend_ratio = atof(optarg);
        break;
      
      case 22:
        opt_svg_showlegend = 0;
        break;

      case 23:
        opt_svg_marginleft = atol(optarg);
        break;

      case 24:
        opt_svg_marginright = atol(optarg);
        break;

      case 25:
        opt_svg_margintop = atol(optarg);
        break;

      case 26:
        opt_svg_marginbottom = atol(optarg);
        break;

      case 27:
        opt_svg_inner_radius = atol(optarg);
        break;

      case 28:
        opt_precision = atoi(optarg);
        break;

      case 29:
        opt_prune_random = atol(optarg);
        break;

      case 30:
        opt_seed = atol(optarg);
        break;

      case 31:
        opt_subtree_short = atof(optarg);
        if (opt_subtree_short < 0)
          fatal("Error: --subtree_short must be a positive real number");
        break;

      case 32:
        opt_info = 1;
        break;

      case 33:
        opt_make_binary = 1;
        break;

      default:
        fatal("Internal error in option parsing");
    }
  }

  if (c != -1)
    exit(EXIT_FAILURE);

  int commands  = 0;

  /* check for mandatory options */
  if (opt_treefile)
  {
    mand_options++;
  }

  /* check for number of independent commands selected */
  if (opt_version)
  {
    mand_options = mandatory_options_count;
    commands++;
  }
  if (opt_help)
  {
    mand_options = mandatory_options_count;
    commands++;
  }
  if (opt_lca_left)
    commands++;
  if (opt_lca_right)
    commands++;
  if (opt_identical)
    commands++;
  if (opt_root)
    commands++;
  if (opt_extract_ltips)
    commands++;
  if (opt_extract_rtips)
    commands++;
  if (opt_extract_tips)
    commands++;
  if (opt_prune_tips)
    commands++;
  if (opt_prune_random)
    commands++;
  if (opt_svg)
    commands++;
  if (opt_extract_lsubtree)
    commands++;
  if (opt_extract_rsubtree)
    commands++;
  if (opt_treeshow)
    commands++;
  if (opt_induce_subtree)
    commands++;
  if (opt_subtree_short >= 0)
    commands++;
  if (opt_info)
    commands++;
  if (opt_make_binary)
    commands++;

  /* if more than one independent command, fail */
  if (commands > 1)
    fatal("More than one command specified");

  /* if no command specified, turn on --help */
  if (!commands)
  {
    opt_help = 1;
    return;
  }

  /* check for mandatory options */
  if (mand_options != mandatory_options_count)
    fatal("Mandatory options are:\n\n%s", mandatory_options_list);

}

void cmd_help()
{
  fprintf(stderr,
          "Usage: %s [OPTIONS]\n", progname);
  fprintf(stderr,
          "\n"
          "General options:\n"
          "  --help                         Display help information.\n"
          "  --version                      Display version information.\n"
          "  --quiet                        Only output warnings and fatal errors to stderr.\n"
          "  --precision                    Number of digits to display after decimal point.\n"
          "  --seed INT                     Seed to initialize random number generator.\n"
          "Commnads for binary trees:\n"
          "  --lca_left                     Print  two  taxa whose LCA is the left child of\n"
          "                                 the root node.\n"
          "  --lca_right                    Print  two taxa whose LCA is the right child of\n"
          "                                 the root node.\n"
          "  --identical FILENAME           Check whether the tree specified by FILENAME is\n"
          "                                 identical to the --tree_file.\n"
          "  --extract_ltips                Display all tip label of left subtree.\n"
          "  --extract_rtips                Display all tip label of right subtree.\n"
          "  --svg                          Create an SVG image of the tree.\n"
          "  --induce_subtree TAXA          Construct induced tree from specified taxa.\n"
          "  --subtree_short REAL           Print all subtrees where all branch lengths are\n"
          "                                 shorter or equal to the threshold\n"
          "Commands for unrooted trees:\n"
          "  --root TAXA                    Root  the  tree  on  the  outgroup specified by\n"
          "                                 TAXA. The edge connecting the outgroup (must be\n"
          "                                 a  subtree)  with the rest of the tree is split\n"
          "                                 into  two edges and a new root node is created.\n"
          "                                 If  TAXA  is empty, then the longest tip-branch\n"
          "                                 is used.\n"
          "Commands for all tree types:\n"
          "  --extract_tips                 Display all tip labels.\n"
          "  --prune_tips TAXA              Prune the comma-separated TAXA from the tree.\n"
          "  --prune_random INT             Randomly prune the specified amount of taxa.\n"
          "  --tree_show                    Display an ASCII version of the tree.\n"
          "  --info                         Display information about tree.\n"
          "  --make_binary                  Convert n-ary/unrooted tree to binary.\n"
          "Options for visualization:\n"
          "  --svg_width INT                Width of SVG image in pixels (default: 1920).\n"
          "  --svg_fontsize INT             Font size of SVG image. (default: 12)\n"
          "  --svg_tipspacing INT           Vertical   distance  (in  pixels)  between  two\n"
          "                                 consencutive  taxa  in  the SVG image (default:\n"
          "                                 20).\n"
          "  --svg_legend_ratio <0..1>      Ratio of the total tree length to be displayed as legend line.\n"
          "  --svg_nolegend                 Do not show the legend.\n"
          "  --svg_marginleft INT           Left margin in pixels (default: 20).\n"
          "  --svg_marginright INT          Right margin in pixels (default: 20).\n"
          "  --svg_margintop INT            Top margin in pixels (default: 20).\n"
          "  --svg_marginbottom INT         Bottom margin in pixels (default: 20).\n"
          "  --svg_inner_radius             Radius of inner nodes in pixels (default: 0).\n"
          "Input and output options:\n"
          "  --tree_file FILENAME           tree file in newick format.\n"
          "  --output_file FILENAME         Optional output file name. If not specified, output is displayed on terminal.\n"
         );
}

void cmd_tree_show()
{
  FILE * out;

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

    utree_show_ascii(out,utree);
    utree_destroy(utree);
  }
  else
  {
    if (!opt_quiet)
      fprintf(stdout, "Loaded rooted binary tree...\n");

    rtree_show_ascii(out,rtree);
    rtree_destroy(rtree);
  }

  if (opt_outfile)
    fclose(out);
}

void cmd_lca_left(void)
{
  rtree_t * tip1, * tip2;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
    fatal("Tree must be rooted...\n");
  
  lca_tips(rtree, &tip1, &tip2);

  if (!opt_quiet)
    printf("Computing left lca tips...\n");

  if (tip1)
    fprintf(stdout,"%s\n",tip1->label);
  if (tip2)
    fprintf(stdout,"%s\n",tip2->label);

  /* deallocate tree structure */
  rtree_destroy(rtree);

  if (!opt_quiet)
    fprintf(stdout, "Done...\n");
}

void cmd_root(void)
{
  FILE * out;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  int tip_count;

  utree_t * utree = utree_parse_newick(opt_treefile, &tip_count);
  if (!utree)
    fatal("File %s does not contain an unrooted binary tree...", opt_treefile);

  rtree_t * rtree = utree_convert_rtree(utree, tip_count, opt_root);
  utree_destroy(utree);

  if (!opt_quiet)
    fprintf(stdout, "Writing tree file...\n");

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

void cmd_extract_subtree(int which)
{
  FILE * out;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
    fatal("Tree must be rooted...\n");

  
  char * newick;
  if (which == 0)
    newick = rtree_export_newick(rtree->left);
  else
    newick = rtree_export_newick(rtree->right);

  fprintf(out, "%s\n", newick);

  if (opt_outfile)
    fclose(out);

  free(newick);
  
  /* deallocate tree structure */
  rtree_destroy(rtree);

  if (!opt_quiet)
    fprintf(stdout, "\nDone...\n");
}
void cmd_extract_ltips(void)
{
  unsigned int i;
  FILE * out;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
    fatal("Tree must be rooted...\n");
  
  if (!opt_quiet)
    fprintf(out,"Tip labels for left subtree:\n");

  /* allocate list of tip nodes in left subtree */
  rtree_t ** node_list = (rtree_t **)calloc(rtree->left->leaves,
                                            sizeof(rtree_t *)); 
  rtree_query_tipnodes(rtree->left, node_list);

  /* print tip-node labels */
  for (i = 0; i < rtree->left->leaves; ++i)
    fprintf(out,"%s\n", node_list[i]->label);

  /* deallocate tree structure */
  rtree_destroy(rtree);

  free(node_list);

  if (opt_outfile)
    fclose(out);

  if (!opt_quiet)
    fprintf(stdout, "\nDone...\n");
}

void cmd_extract_rtips(void)
{
  unsigned int i;
  FILE * out;

  /* attempt to open output file */
  out = opt_outfile ?
          xopen(opt_outfile,"w") : stdout;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
    fatal("Tree must be rooted...\n");
  
  if (!opt_quiet)
    fprintf(out,"Tip labels for left subtree:\n");

  /* allocate list of tip nodes in left subtree */
  rtree_t ** node_list = (rtree_t **)calloc(rtree->right->leaves,
                                            sizeof(rtree_t *)); 
  rtree_query_tipnodes(rtree->right, node_list);

  /* print tip-node labels */
  for (i = 0; i < rtree->right->leaves; ++i)
    fprintf(out, "%s\n", node_list[i]->label);

  /* deallocate tree structure */
  rtree_destroy(rtree);

  free(node_list);

  if (opt_outfile)
    fclose(out);

  if (!opt_quiet)
    fprintf(stdout, "\nDone...\n");
}

void cmd_extract_tips(void)
{
  unsigned int i;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (rtree)
  {
    if (!opt_quiet)
      printf("Loaded binary rooted tree...\n");

    if (!opt_quiet)
      printf("Tip labels:\n");

    rtree_t ** node_list = (rtree_t **)calloc(rtree->leaves,sizeof(rtree_t *)); 
    rtree_query_tipnodes(rtree, node_list);

    for (i = 0; i < rtree->leaves; ++i)
      printf("%s\n", node_list[i]->label);

    /* deallocate tree structure */
    rtree_destroy(rtree);

    free(node_list);
  }
  else
  {
    int tip_count;

    utree_t * utree = utree_parse_newick(opt_treefile, &tip_count);
    if (!utree)
      fatal("Tree is neither rooted or unrooted...\n");
    
    if (!opt_quiet)
      printf("Loaded binary unrooted tree...\n");

    if (!opt_quiet)
      printf("Tip labels:\n");

    utree_t ** node_list = (utree_t **)calloc(tip_count,sizeof(utree_t *)); 
    tip_count = utree_query_tipnodes(utree, node_list);

    for (i = 0; (int)i < tip_count; ++i)
      printf("%s\n", node_list[i]->label);

    /* deallocate tree structure */
    utree_destroy(utree);

    free(node_list);
  }
  
  if (!opt_quiet)
    fprintf(stdout, "\nDone...\n");
}

void cmd_identical(void)
{
  int i;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree1 = rtree_parse_newick(opt_treefile);
  rtree_t * rtree2 = rtree_parse_newick(opt_identical);

  if (!rtree1)
    fatal("File %s does not contain a rooted binary tree...", opt_treefile);
  if (!rtree2)
    fatal("File %s does not contain a rooted binary tree...", opt_identical);

  if (rtree1->leaves != rtree2->leaves)
    printf("Trees have different topologies (number of leaves mismatch)\n");
  else
  {
    rtree_t ** node_list1 = (rtree_t **)calloc(2*rtree1->leaves-1,sizeof(rtree_t *)); 
    rtree_t ** node_list2 = (rtree_t **)calloc(2*rtree2->leaves-1,sizeof(rtree_t *)); 

    int index = 0;
    rtree_traverse_sorted(rtree1, node_list1, &index);
    index = 0;
    rtree_traverse_sorted(rtree2, node_list2, &index);

    for(i = 0; i < index; ++i)
    {
      if (node_list1[i]->left == NULL && node_list2[i]->left != NULL)
      {
        printf("Trees have different topologies\n");
        break;
      }
      else if (node_list1[i]->left != NULL && node_list2[i]->left == NULL)
      {
        printf("Trees have different topologies\n");
        break;
      }

      if (!node_list1[i]->label && node_list2[i]->label)
      {
        printf("Trees have different topologies\n");
        break;
      }
      else if (node_list1[i]->label && !node_list2[i]->label)
      {
        printf("Trees have different topologies\n");
        break;
      }
      else if (node_list1[i]->label && node_list2[i]->label)
      {
        if (strcmp(node_list1[i]->label, node_list2[i]->label))
        {
          printf("Trees have different topologies\n");
          break;
        }
      }
    }

    if (i == index)
      printf("Trees have identical topologies\n");

    free(node_list1);
    free(node_list2);
  }

  /* deallocate tree structure */
  rtree_destroy(rtree1);
  rtree_destroy(rtree2);

  if (!opt_quiet)
    fprintf(stdout, "Done...\n");
  
}

void cmd_make_binary()
{
  FILE * out;

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (rtree)
  {
    printf("Loaded tree is already binary...\n");

    /* deallocate tree structure */
    rtree_destroy(rtree);
  }
  else
  {
    int tip_count;

    utree_t * utree = utree_parse_newick(opt_treefile, &tip_count);
    if (utree)
    {
      if (!opt_quiet)
        printf("Loaded unrooted binary tree...\n");

      /* deallocate tree structure */
      utree_destroy(utree);
    }

    /* load it as n-ary */
    ntree_t * ntree = ntree_parse_newick(opt_treefile);
    if (ntree)
    {
      /* convert to binary */
      rtree_t * rt = ntree_to_rtree(ntree);

      /* attempt to open output file */
      out = opt_outfile ?
              xopen(opt_outfile,"w") : stdout;

      char * newick = rtree_export_newick(rt);

      if (!opt_quiet)
        fprintf(stdout,"Writing newick string...\n");
      fprintf(out, "%s\n", newick);

      free(newick);

      fclose(out);

      /* deallocate tree structures */
      ntree_destroy(ntree);
      rtree_destroy(rt);
    }
    else
      fatal("Failed loading tree as n-ary");
  }
}

void getentirecommandline(int argc, char * argv[])
{
  int len = 0;
  int i;

  for (i = 0; i < argc; ++i)
    len += strlen(argv[i]);

  cmdline = (char *)xmalloc(len + argc + 1);
  cmdline[0] = 0;

  for (i = 0; i < argc; ++i)
  {
    strcat(cmdline, argv[i]);
    strcat(cmdline, " ");
  }
}

void fillheader()
{
  snprintf(progheader, 80,
           "%s %s_%s, %1.fGB RAM, %ld cores",
           PROG_NAME, PROG_VERSION, PROG_ARCH,
           arch_get_memtotal() / 1024.0 / 1024.0 / 1024.0,
           sysconf(_SC_NPROCESSORS_ONLN));
}

void show_header()
{
  fprintf(stdout, "%s\n", progheader);
  fprintf(stdout, "https://github.com/xflouris/newick-tools\n");
  fprintf(stdout,"\n");
}

int main (int argc, char * argv[])
{
  fillheader();
  getentirecommandline(argc, argv);

  args_init(argc, argv);
  
  srand(opt_seed);

  if (!opt_quiet)
    show_header();

  if (opt_help)
  {
    cmd_help();
  }
  else if (opt_lca_left)
  {
    cmd_lca_left();
  }
  else if (opt_identical)
  {
    cmd_identical();
  }
  else if (opt_root)
  {
    cmd_root();
  }
  else if (opt_extract_ltips)
  {
    cmd_extract_ltips();
  }
  else if (opt_extract_rtips)
  {
    cmd_extract_rtips();
  }
  else if (opt_extract_tips)
  {
    cmd_extract_tips();
  }
  else if (opt_prune_tips || opt_prune_random)
  {
    cmd_prune_tips();
  }
  else if (opt_svg)
  {
    cmd_svg();
  }
  else if (opt_extract_lsubtree)
  {
    cmd_extract_subtree(0);
  }
  else if (opt_extract_rsubtree)
  {
    cmd_extract_subtree(1);
  }
  else if (opt_treeshow)
  {
    cmd_tree_show();
  }
  else if (opt_induce_subtree)
  {
    cmd_induce_tree();
  }
  else if (opt_subtree_short >= 0)
  {
    cmd_subtree_short();
  }
  else if (opt_info)
  {
    cmd_info();
  }
  else if (opt_make_binary)
  {
    cmd_make_binary();
  }

  free(cmdline);
  return (0);
}
