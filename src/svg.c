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


double scaler = 0;
long stroke_width = 3;

long legend_spacing = 10;
double opt_svg_legend_ratio = 0.3;

FILE * svg_fp;

double max_font_len = 0;
double max_tree_len = 0;

double canvas_height;
double canvas_width;

static int tip_count;

typedef struct coord_s 
{
  double x;
  double y;
} coord_t; 

static coord_t * create_coord(double x, double y)
{
  coord_t * coord = (coord_t *)xmalloc(sizeof(coord_t));
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

static void svg_circle(double cx, double cy, double r)
{
  fprintf(svg_fp,
          "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"#31a354\" "
          "stroke=\"#31a354\" />\n",
          cx, cy, r);
}

static void utree_set_xcoord(utree_t * node)
{
  utree_t * parent = NULL;

  /* create the coordinate info of the node's scaled branch length (edge
     towards root) */
  coord_t * coord = create_coord(node->length * scaler, 0);
  node->data = (void *)coord;
  
  /* now set this info to all nodes in the round-about structure */
  if (node->next)
  {
    node->next->data = (void *)coord;
    node->next->next->data = (void *)coord;
  }
  if (node->back->height > node->height)
    parent = node->back;

  /* if the node has a parent then add the x coord of the parent such that
     the branch is shifted towards right, otherwise, if the node is the root,
     align it with the left margin */
  if (parent)
    coord->x += ((coord_t *)(parent->data))->x;
  else
    coord->x = opt_svg_marginleft;

  if (!node->next) 
    return;
  
  /* recursively set coordinates of the other nodes in a pre-order fashion */
  utree_set_xcoord(node->next->back);
  utree_set_xcoord(node->next->next->back);
  if (!parent)
    utree_set_xcoord(node->back);
}

static void rtree_set_xcoord(rtree_t * node)
{
  /* create the coordinate info of the node's scaled branch length (edge
     towards root) */
  coord_t * coord = create_coord(node->length * scaler, 0);
  node->data = (void *)coord;

  /* if the node has a parent then add the x coord of the parent such that
     the branch is shifted towards right, otherwise, if the node is the root,
     align it with the left margin */
  if (node->parent)
    coord->x += ((coord_t *)(node->parent->data))->x;
  else
  {
    coord->x = opt_svg_marginleft;
  }

  if (!node->left) 
    return;
  
  /* recursively set coordinates of the other nodes in a pre-order fashion */
  rtree_set_xcoord(node->left);
  rtree_set_xcoord(node->right);
}

static void svg_utree_plot(utree_t * node)
{
  double y;
  static int tip_occ = 0;
  utree_t * parent = NULL;

  if (node->back->height > node->height)
    parent = node->back;

  if (node->next)
  {
    svg_utree_plot(node->next->back);
    svg_utree_plot(node->next->next->back);
    if (!parent)
      svg_utree_plot(node->back);
      
  }

  if (parent)
  {
    double x,px;

    x = ((coord_t *)(node->data))->x;
    px = ((coord_t *)(parent->data))->x;


    if (!node->next)
    {
      y = tip_occ * opt_svg_tipspace + opt_svg_margintop + legend_spacing;
      tip_occ++;
      
    }
    else
    {
      double ly,ry;
      ly = ((coord_t *)(node->next->back->data))->y;
      ry = ((coord_t *)(node->next->next->back->data))->y;
      y = (ly + ry) / 2.0;


      svg_line(x,
               ly,
               x,
               ry,
               stroke_width);
      
      svg_circle(x,y,opt_svg_inner_radius);
    }
    /* horizontal line */
    svg_line(px,
             y,
             x,
             y,
             stroke_width);
    ((coord_t *)(node->data))->y = y;

    if (!node->next)
    {
      fprintf(svg_fp, "<text x=\"%f\" y=\"%f\" "
                      "font-size=\"%ld\" font-family=\"Arial;\">%s</text>\n",
              x+5,
              y+opt_svg_fontsize/3.0,
              opt_svg_fontsize,
              node->label);
    }
    else
      fprintf(svg_fp, "\n");
  }
  else
  {
    double ly,ry,x;
    ly = ((coord_t *)(node->next->back->data))->y;
    ry = ((coord_t *)(node->back->data))->y;
    y = (ly + ry) / 2.0;
    x = opt_svg_marginleft;


    svg_line(x,
             ly,
             x,
             ry,
             stroke_width);
    svg_circle(x,y,opt_svg_inner_radius);
  }
}

static void svg_rtree_plot(rtree_t * node)
{
  double y;
  static int tip_occ = 0;

  /* traverse tree in post-order */
  if (node->left)
  {
    svg_rtree_plot(node->left);
    svg_rtree_plot(node->right);
  }

  if (node->parent)
  {
    double x,px;

    x = ((coord_t *)(node->data))->x;
    px = ((coord_t *)(node->parent->data))->x;


    if (!node->left)
    {
      y = tip_occ * opt_svg_tipspace + opt_svg_margintop + legend_spacing;
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
      
      svg_circle(x,y,opt_svg_inner_radius);
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
              y+opt_svg_fontsize/3.0,
              opt_svg_fontsize,
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
    x = opt_svg_marginleft;


    svg_line(x,
             ly,
             x,
             ry,
             stroke_width);
    svg_circle(x,y,opt_svg_inner_radius);
  }
}

static void utree_scaler_init(utree_t * root, int tip_count)
{
  double len = 0;
  double label_len;
  int i;

  utree_t ** node_list = (utree_t **)malloc(tip_count*sizeof(utree_t *));

  utree_query_tipnodes(root, node_list);

  /* compute the length of all tip-to-root paths and store the longest one in
     max_tree_len */
  for (i = 0; i < tip_count; ++i)
  {
    utree_t * node = node_list[i];

    len = node->length;
    node = node->back;
    while(1)
    {
      if (node->next->back->height > node->height)
        node = node->next->back;
      else if (node->next->next->back->height > node->height)
        node = node->next->next->back;
      else
        break;

      len += node->length;
    }
    
    if (len > max_tree_len) 
      max_tree_len = len;

    label_len = (opt_svg_fontsize / 1.5) * 
                (node_list[i]->label ? strlen(node_list[i]->label) : 0);

    len = (canvas_width - label_len) / len;
    if (i == 0)
    {
      scaler = len;
      max_font_len = label_len;
    }
    else
      if (len < scaler)
      {
        scaler = len;
        max_font_len = label_len;
      }
  }
  free(node_list);
}

static void rtree_scaler_init(rtree_t * root)
{
  double len = 0;
  double label_len;
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
    /* subtract root length */
    len -= root->length;

    if (len > max_tree_len) 
      max_tree_len = len;

    label_len = (opt_svg_fontsize / 1.5) * 
                (node_list[i]->label ? strlen(node_list[i]->label) : 0);

    len = (canvas_width - label_len) / len;
    if (i == 0)
    {
      scaler = len;
      max_font_len = label_len;
    }
    else
      if (len < scaler)
      {
        scaler = len;
        max_font_len = label_len;
      }
  }
  free(node_list);
}

void svg_utree_init(utree_t * root, int tip_count)
{
  long svg_height;

  canvas_width = opt_svg_width - opt_svg_marginleft - opt_svg_marginright;

  /* initialize pixel scaler (scaler) and compute max tree 
     length (max_tree_len) */
  utree_scaler_init(root, tip_count);

  svg_height = opt_svg_margintop + legend_spacing + opt_svg_marginbottom + 
               opt_svg_tipspace * tip_count;


  /* print svg header tag with dimensions and grey border */
  fprintf(svg_fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%ld\" "
          "height=\"%ld\" style=\"border: 1px solid #cccccc;\">\n",
          opt_svg_width,
          svg_height);

  /* draw legend */
  if (opt_svg_showlegend)
  {
    svg_line(opt_svg_marginleft,
             10,
             (canvas_width - max_font_len)*opt_svg_legend_ratio + 
                                           opt_svg_marginleft,
             10,
             3);

    fprintf(svg_fp, "<text x=\"%f\" y=\"%f\" font-size=\"%ld\" "
            "font-family=\"Arial;\">%.*f</text>\n",
            (canvas_width - max_font_len)*opt_svg_legend_ratio + opt_svg_marginleft + 5,
            20-opt_svg_fontsize/3.0,
            (long)opt_svg_fontsize, opt_precision, max_tree_len * opt_svg_legend_ratio);
  }

  /* uncomment to print a dashed border to indicate margins */
  
  /*
  fprintf(svg_fp, "<rect x=\"%ld\" y=\"%ld\" width=\"%ld\" fill=\"none\" "
          "height=\"%ld\" stroke=\"#999999\" stroke-dasharray=\"5,5\" "
          "stroke-width=\"1\" />\n",
          opt_svg_marginleft, 
          opt_svg_margintop + legend_spacing, 
          svg_width - opt_svg_marginleft - opt_svg_marginright,
          svg_height - opt_svg_margintop - legend_spacing - opt_svg_marginbottom);
  */
  
  utree_set_xcoord(root);

  svg_utree_plot(root);

  fprintf(svg_fp, "</svg>\n");
}

void svg_rtree_init(rtree_t * root)
{
  long svg_height;

  canvas_width = opt_svg_width - opt_svg_marginleft - opt_svg_marginright;

  /* initialize pixel scaler (scaler) and compute max tree 
     length (max_tree_len) */
  rtree_scaler_init(root);

  svg_height = opt_svg_margintop + legend_spacing + opt_svg_marginbottom + 
               opt_svg_tipspace * root->leaves;


  tip_count = root->leaves;

  /* print svg header tag with dimensions and grey border */
  fprintf(svg_fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%ld\" "
          "height=\"%ld\" style=\"border: 1px solid #cccccc;\">\n",
          opt_svg_width,
          svg_height);

  /* draw legend */
  if (opt_svg_showlegend)
  {
    svg_line(opt_svg_marginleft,
             10,
             (canvas_width - max_font_len)*opt_svg_legend_ratio + 
                                           opt_svg_marginleft,
             10,
             3);

    fprintf(svg_fp, "<text x=\"%f\" y=\"%f\" font-size=\"%ld\" "
            "font-family=\"Arial;\">%.*f</text>\n",
            (canvas_width - max_font_len)*opt_svg_legend_ratio + opt_svg_marginleft + 5,
            20-opt_svg_fontsize/3.0,
            (long)opt_svg_fontsize, opt_precision, max_tree_len * opt_svg_legend_ratio);
  }

  /* uncomment to print a dashed border to indicate margins */
  
  /*
  fprintf(svg_fp, "<rect x=\"%ld\" y=\"%ld\" width=\"%ld\" fill=\"none\" "
          "height=\"%ld\" stroke=\"#999999\" stroke-dasharray=\"5,5\" "
          "stroke-width=\"1\" />\n",
          opt_svg_marginleft, 
          opt_svg_margintop + legend_spacing, 
          svg_width - opt_svg_marginleft - opt_svg_marginright,
          svg_height - opt_svg_margintop - legend_spacing - opt_svg_marginbottom);
  */
  
  rtree_set_xcoord(root);

  svg_rtree_plot(root);

  fprintf(svg_fp, "</svg>\n");
}


void cmd_svg(void)
{
  utree_t * utree = NULL;
  int tip_count;

  if (!opt_outfile)
    fatal("An output file must be specified");

  /* parse tree */
  if (!opt_quiet)
    fprintf(stdout, "Parsing tree file...\n");

  rtree_t * rtree = rtree_parse_newick(opt_treefile);

  if (!rtree)
  {
    utree = utree_parse_newick(opt_treefile, &tip_count);
    if (!utree)
      fatal("Tree is neither unrooted nor rooted. Go fix your tree.");
     
    if (!opt_quiet)
      fprintf(stdout, "Loaded unrooted tree...\n");
  }
  else
  {
    if (!opt_quiet)
      fprintf(stdout, "Loaded rooted tree...\n");
  }
  
  if (!opt_quiet)
    printf("Creating SVG...\n");

  svg_fp = fopen(opt_outfile, "w");
  if (!svg_fp)
    fatal("Cannot write to file %s", opt_outfile);


  if (rtree)
    svg_rtree_init(rtree);
  else
    svg_utree_init(utree, tip_count);

  fclose(svg_fp);

  /* deallocate tree structure */
  if (utree)
    utree_destroy(utree);
  else
    rtree_destroy(rtree);

  if (!opt_quiet)
    fprintf(stdout, "\nDone...\n");
}
