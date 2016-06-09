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

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <search.h>
#include <getopt.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdbool.h>

/* constants */

#define PROG_NAME "newick-tools"
#define PROG_VERSION "v0.0.0"

#ifdef __APPLE__
#define PROG_ARCH "macosx_x86_64"
#else
#define PROG_ARCH "linux_x86_64"
#endif

/* structures and data types */

typedef unsigned int UINT32;
typedef unsigned short WORD;
typedef unsigned char BYTE;

typedef struct utree_s
{
  char * label;
  double length;
  int height;
  struct utree_s * next;
  struct utree_s * back;
  int mark;

  void * data;
} utree_t;

typedef struct rtree_s
{
  char * label;
  double length;
  struct rtree_s * left;
  struct rtree_s * right;
  struct rtree_s * parent;
  unsigned int leaves;
  char * color;
  int mark;

  void * data;
} rtree_t;

typedef struct ntree_s
{
  char * label;
  double length;
  struct ntree_s ** children;
  struct ntree_s * parent;
  int children_count;
  int mark;
} ntree_t;

/* macros */

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* options */

extern int opt_quiet;
extern int opt_precision;
extern int opt_svg_showlegend;
extern char * opt_treefile;
extern char * opt_identical;
extern char * opt_outfile;
extern char * opt_prune_tips;
extern char * opt_root;
extern char * opt_induce_subtree;
extern char * opt_alltree_filename;
extern long opt_help;
extern long opt_version;
extern long opt_treeshow;
extern long opt_lca_left;
extern long opt_lca_right;
extern long opt_extract_ltips;
extern long opt_extract_rtips;
extern long opt_extract_tips;
extern long opt_prune_random;
extern long opt_info;
extern long opt_make_binary;
extern long opt_svg;
extern long opt_seed;
extern long opt_extract_lsubtree;
extern long opt_extract_rsubtree;
extern long opt_svg_width;
extern long opt_svg_fontsize;
extern long opt_svg_tipspace;
extern long opt_svg_marginleft;
extern long opt_svg_marginright;
extern long opt_svg_margintop;
extern long opt_svg_marginbottom;
extern long opt_svg_inner_radius;
extern double opt_svg_legend_ratio;
extern double opt_subtree_short;

/* common data */

extern char errmsg[200];

extern long mmx_present;
extern long sse_present;
extern long sse2_present;
extern long sse3_present;
extern long ssse3_present;
extern long sse41_present;
extern long sse42_present;
extern long popcnt_present;
extern long avx_present;
extern long avx2_present;

/* functions in util.c */

void fatal(const char * format, ...);
void progress_init(const char * prompt, unsigned long size);
void progress_update(unsigned int progress);
void progress_done();
void * xmalloc(size_t size);
void * xrealloc(void *ptr, size_t size);
char * xstrchrnul(char *s, int c);
char * xstrdup(const char * s);
char * xstrndup(const char * s, size_t len);
long getusec(void);
void show_rusage();
FILE * xopen(const char * filename, const char * mode);
void shuffle(void * array, size_t n, size_t size);

/* functions in newick-tools.c */

void args_init(int argc, char ** argv);
void cmd_help();
void getentirecommandline(int argc, char * argv[]);
void fillheader();
void show_header();
void cmd_tree_show();

/* functions in parse_ntree.y */

ntree_t * ntree_parse_newick(const char * filename);

void ntree_destroy(ntree_t * root);

/* functions in parse_rtree.y */

rtree_t * rtree_parse_newick(const char * filename);

void rtree_destroy(rtree_t * root);

/* functions in parse_utree.y */

utree_t * utree_parse_newick(const char * filename,
                             int * tip_count);

void utree_destroy(utree_t * root);

/* functions in ntree.c */

void ntree_node_count(ntree_t * root,
                      int * inner_count,
                      int * tip_count,
                      int * min_inner_degree,
                      int * max_inner_degree);

rtree_t * ntree_to_rtree(ntree_t * root);

/* functions in utree.c */

void utree_show_ascii(FILE * stream, utree_t * tree);

char * utree_export_newick(utree_t * root);

int utree_traverse(utree_t * root,
                   int (*cbtrav)(utree_t *),
                   utree_t ** outbuffer);

int utree_query_tipnodes(utree_t * root,
                         utree_t ** node_list);

int utree_query_innernodes(utree_t * root,
                           utree_t ** node_list);

rtree_t * utree_convert_rtree(utree_t * root,
                              int tip_count,
                              char * outgroup_list);

utree_t ** utree_tipstring_nodes(utree_t * root,
                                 unsigned int tips_count,
                                 char * tipstring,
                                 unsigned int * tiplist_count);

/* functions in rtree.c */

void rtree_show_ascii(FILE * stream, rtree_t * tree);

char * rtree_export_newick(rtree_t * root);

int rtree_traverse(rtree_t * root,
                   int (*cbtrav)(rtree_t *),
                   rtree_t ** outbuffer);

int rtree_query_tipnodes(rtree_t * root,
                         rtree_t ** node_list);

int rtree_query_innernodes(rtree_t * root,
                           rtree_t ** node_list);

void rtree_reset_leaves(rtree_t * root);

char * rtree_label(rtree_t * root);

void rtree_traverse_sorted(rtree_t * root, rtree_t ** node_list, int * index);

rtree_t ** rtree_tipstring_nodes(rtree_t * root,
                                 char * tipstring,
                                 unsigned int * tiplist_count);

rtree_t ** rtree_tiplist_complement(rtree_t * root,
                                    rtree_t ** tiplist,
                                    unsigned int tiplist_count);

int rtree_traverse_postorder(rtree_t * root,
                             int (*cbtrav)(rtree_t *),
                             rtree_t ** outbuffer);

/* functions in parse_rtree.y */

rtree_t * rtree_parse_newick(const char * filename);

/* functions in arch.c */

unsigned long arch_get_memused();
unsigned long arch_get_memtotal();

/* functions in lca_tips.c */

void lca_tips(rtree_t * root, rtree_t ** tip1, rtree_t ** tip2);

/* functions in lca_utree.c */

void lca_init(utree_t * root);
void lca_destroy();
utree_t * lca_compute(utree_t * tip1, utree_t * tip2);

/* functions in utree_bf.c */
void cmd_utree_bf(void);

/* functions in prune.c */

void cmd_prune_tips(void);
void cmd_induce_tree(void);

/* functions in svg.c */

void cmd_svg(void);

/* functions in subtree.c */
void cmd_subtree_short(void);

/* functions in info.c */
void cmd_info(void);
