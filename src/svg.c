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


long svg_width  = 1920;
long svg_height = 850;

/* percentage */
long svg_margin_top = 20;
long svg_margin_bottom = 20;
long svg_margin_left = 20;
long svg_margin_right = 20;

double viewport_height;

double max_path_size = 0;
long tip_spacing = 20;
long scaler;
long stroke_width = 3;
long font_size = 12;

long legend_spacing = 10;
double legend_line_ratio = 0.3;

FILE * svg_fp;

double max_font_len = 0;
double max_tree_len = 0;

static int tip_count;

typedef struct coord_s 
{
  double x;
  double y;
} coord_t; 

static coord_t * create_coord(double x, double y)
{
  coord_t * coord = (coord_t *)xmalloc(sizeof(coord));
  coord->x = x;
  coord->y = y;
  return coord;
}

static void svg_line(double x1, double y1, double x2, double y2, double stroke_width)
{
  fprintf(svg_fp,
          "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" "
          "stroke=\"#31a354\" stroke-width=\"%f\" />\n",
          x1, y1, x2, y2, stroke_width);
}

static void set_xcoord(rtree_t * node)
{
  //coord_t * coord = create_coord(node->length * (svg_width - svg_margin_left - svg_margin_right) / max_path_size ,0);
  coord_t * coord = create_coord(node->length * max_path_size, 0);
  node->data = (void *)coord;
  if (node->parent)
    coord->x += ((coord_t *)(node->parent->data))->x;
  else
  {
    coord->x = svg_margin_left;
  }

  if (!node->left) 
    return;
  
  set_xcoord(node->left);
  set_xcoord(node->right);
}

static void draw_tree_abs(rtree_t * node)
{
  double y;
  static int tip_occ = 0;

  if (node->left)
  {
    draw_tree_abs(node->left);
    draw_tree_abs(node->right);
  }

  if (node->parent)
  {
    double x,px;

    x = ((coord_t *)(node->data))->x;
    px = ((coord_t *)(node->parent->data))->x;


    if (!node->left)
    {
      y = tip_occ * tip_spacing + svg_margin_top + legend_spacing;
      tip_occ++;
      
    }
    else
    {
      double ly,ry;
      ly = ((coord_t *)(node->left->data))->y;
      ry = ((coord_t *)(node->right->data))->y;
      y = (ly + ry) / 2.0;


      svg_line(x,
               ly,
               x,
               ry,
               stroke_width);
    }
    /* horizontal line */
    svg_line(px,
             y,
             x,
             y,
             stroke_width);
    ((coord_t *)(node->data))->y = y;

    if (!node->left)
    {
      fprintf(svg_fp, "<text x=\"%f\" y=\"%f\" "
                      "font-size=\"%ld\" font-family=\"Arial;\">%s</text>\n",
              x+5,
              y+font_size/3.0,
              font_size,
              node->label);
    }
    else
      fprintf(svg_fp, "\n");
  }
  else
  {
    double ly,ry,x;
    //    lx = ((coord_t *)(node->left->data))->x;
    ly = ((coord_t *)(node->left->data))->y;
    //    rx = ((coord_t *)(node->right->data))->x;
    ry = ((coord_t *)(node->right->data))->y;
    y = (ly + ry) / 2.0;
    x = svg_margin_left;


    svg_line(x,
             ly,
             x,
             ry,
             stroke_width);
  }
}


static double longest_path_length(rtree_t * root)
{
  double max_len = 0;
  double len = 0;
  unsigned int i;

  rtree_t ** node_list = (rtree_t **)malloc((2*root->leaves-1)*sizeof(rtree_t *));

  rtree_query_tipnodes(root, node_list);

  /* find longest path to root */

  for (i = 0; i < root->leaves; ++i)
  {
    rtree_t * node = node_list[i];

    len = 0;
    while(node)
    {
      len += node->length;
      node = node->parent;
    }
    len -= root->length;
    if (len > max_tree_len) 
      max_tree_len = len;

    len = (svg_width - svg_margin_left - svg_margin_right - 
          (node_list[i]->label ? strlen(node_list[i]->label) : 0) * 
          (font_size/1.5)) / len;
    if (i == 0)
      max_len = len;
    else
      if (len < max_len)
      {
        max_len = len;
        max_font_len = (node_list[i]->label ? strlen(node_list[i]->label) : 0)*
                       (font_size/1.5);
      }
  }
  free(node_list);

  return max_len;
}

void svg_init(rtree_t * root)
{
  max_path_size = longest_path_length(root);

  viewport_height = svg_height;
  svg_height = svg_margin_top + legend_spacing + svg_margin_bottom + tip_spacing*root->leaves;

  tip_count = root->leaves;

  fprintf(svg_fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%ld\" "
          "height=\"%ld\" style=\"border: 1px solid #cccccc;\">\n",
          svg_width,
          svg_height);

  fprintf(svg_fp, "<line x1=\"20\" y1=\"10\" x2=\"%f\" y2=\"10\" "
          "stroke=\"#31a354\" stroke-width=\"3\" />\n",
          (svg_width - svg_margin_right - svg_margin_left- max_font_len)*legend_line_ratio + 20);
  fprintf(svg_fp, "<text x=\"%f\" y=\"%f\" font-size=\"%ld\" "
          "font-family=\"Arial;\">%.*f</text>\n",
          (svg_width - svg_margin_right - svg_margin_left - max_font_len)*legend_line_ratio + 20 + 5,
          20-font_size/3.0,
          (long)font_size, opt_precision, max_tree_len * legend_line_ratio);

  /* uncomment to print a dashed border to indicate margins */
  
  fprintf(svg_fp, "<rect x=\"%ld\" y=\"%ld\" width=\"%ld\" fill=\"none\" "
          "height=\"%ld\" stroke=\"#999999\" stroke-dasharray=\"5,5\" "
          "stroke-width=\"1\" />\n",
          svg_margin_left, 
          svg_margin_top + legend_spacing, 
          svg_width - svg_margin_left - svg_margin_right,
          svg_height - svg_margin_top - legend_spacing - svg_margin_bottom);
  
  set_xcoord(root);

  draw_tree_abs(root);

  fprintf(svg_fp, "</svg>\n");
}


void cmd_svg(void)
{
  if (!opt_outfile)
    fatal("An output file must be specified");

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
    fatal("Tree must be rooted...\n");
  
  if (!opt_quiet)
    printf("Creating SVG...\n");

  svg_fp = fopen(opt_outfile, "w");
  if (!svg_fp)
    fatal("Cannot write to file %s", opt_outfile);


  /* set svg parameters from input */
  svg_width = opt_svg_width;
  font_size = opt_svg_fontsize;
  tip_spacing = opt_svg_tipspace;
  legend_line_ratio = opt_svg_legend_ratio;

  svg_init(rtree);

  fclose(svg_fp);

  /* deallocate tree structure */
  rtree_destroy(rtree);

  if (!opt_quiet)
    fprintf(stdout, "\nDone...\n");
}
