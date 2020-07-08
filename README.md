# newick-tools: a novel software for simulating and processing phylogenetic trees

[![License](https://img.shields.io/badge/license-AGPL-blue.svg)](http://www.gnu.org/licenses/agpl-3.0.en.html)
[![Build Status](https://travis-ci.org/xflouris/newick-tools.svg?branch=master)](https://magnum.travis-ci.com/xflouris/newick-tools)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1312910.svg)](https://doi.org/10.5281/zenodo.1312909)

## Introduction

The aim of this project is to implement a proper, multifunctional newick
manipulation toolkit called `newick-tools`. The toolkit should:

* correctly parse newick files, both binary rooted and binary unrooted.
* parse n-ary newick files and allow collapsing into binary rooted/unrooted.
* parse the [newick extended format](http://dmi.uib.es/~gcardona/BioInfo/enewick.html)
* create new topologies from existing one by pruning taxa, or inducing subtrees.
* generate topologies
* list taxa, or taxa of specific subtrees
* visualize the tree in terminal (ASCII), vector formats such as SVG and EPS, and raster format such as PNG.
* compare topologies
* root trees given outgroup taxon or outgroup subtree
* locate repeated substructures (subtree repeats)
* display tree info (rooting,number of taxa, max branch length, average branch length etc)
* generate a consensus tree from a collection of trees.
* perform all above functions on files that contain more than one tree, e.g. induce the subtrees of specific taxa in a collection of trees.


## Compilation instructions

Currently, `newick-tools` requires that [GNU Bison](http://www.gnu.org/software/bison/)
and [Flex](http://flex.sourceforge.net/) are installed on the target system. On
a Debian-based Linux system, the two packages can be installed using the command

`apt-get install flex bison`

`newick-tools` also requires that a GNU system is available as it uses several
functions (e.g. `asprintf`) which are not present in the POSIX standard.
This, however may change in the future such that the code is more portable.

`newick-tools` can be compiled using the included Makefile:

`make`

## Command-line options

General options:

* `--help`
* `--version`
* `--quiet`
* `--precision`
* `--seed`

Options for binary trees:
* `--lca_left`
* `--lca_right`
* `--identical`
* `--extract_ltips`
* `--extract_rtips`
* `--svg`
* `--induce_subtree`
* `--subtree_short`
* `--svg_legend_ratio`

Options for unrooted trees:
* `--root`

Options for all tree types:
* `--extract_tips`
* `--prune_tips`
* `--prune_random`
* `--tree_show`
* `--make_binary`
* `--info`

Options for visualization:
* `--svg_width`
* `--svg_fontsize`
* `--svg_tipspacing`
* `--svg_legend_ratio`
* `--svg_nolegend`
* `--svg_marginleft`
* `--svg_marginright`
* `--svg_margintop`
* `--svg_marginbottom`
* `--svg_inner_radius`

Input and output options:
* `--tree_file`
* `--output_file`

## 

## License and third party licenses

The code is currently licensed under the [GNU Affero General Public License version 3](http://www.gnu.org/licenses/agpl-3.0.en.html).

## Code

|                    File                     |                                  Description                                  |
|---------------------------------------------|-------------------------------------------------------------------------------|
|[**``newick-tools.c``**](src/newick-tools.c) | Main file handling command-line parameters and executing corresponding parts. |
|[**``Makefile``**](src/Makefile)             | Makefile.                                                                     |
|[**``lex_rtree.l``**](src/lex_rtree.l)       | Lexical analyzer parsing newick rooted trees.                                 |
|[**``lex_utree.l``**](src/lex_utree.l)       | Lexical analyzer parsing newick unrooted trees.                               |
|[**``lex_ntree.l``**](src/lex_ntree.l)       | Lexical analyzer parsing newick n-ary trees.                                  |
|[**``util.c``**](src/util.c)                 | Various common utility functions.                                             |
|[**``arch.c``**](src/arch.c)                 | Architecture specific code (Mac/Linux).                                       |
|[**``rtree.c``**](src/rtree.c)               | Rooted tree manipulation functions.                                           |
|[**``utree.c``**](src/utree.c)               | Unrooted tree manipulation functions.                                         |
|[**``ntree.c``**](src/ntree.c)               | n-ary tree manipulation functions.                                            |
|[**``parse_rtree.y``**](src/parse_rtree.y)   | Functions for parsing rooted trees in newick format.                          |
|[**``parse_utree.y``**](src/parse_utree.y)   | Functions for parsing unrooted trees in newick format.                        |
|[**``parse_ntree.y``**](src/parse_ntree.y)   | Functions for parsing n-ary trees in newick format.                           |
|[**``lca_utree.c``**](src/lca_utree.c)       | Naive LCA computation in unrooted trees.                                      |
|[**``lca_tips.c``**](src/lca_tips.c)         | Compute tips leading to an LCA node.                                          |
|[**``svg.c``**](src/svg.c)                   | SVG output routines.                                                          |
|[**``prune.c``**](src/prune.c)               | Methods for pruning taxa and inducing subtrees.                               |
|[**``info.c``**](src/info.c)                 | Functions for showing various tree-related  information.                      |

## Bugs

The source code in the master branch is thoroughly tested before commits.
However, mistakes may happen. All bug reports are highly appreciated.


## The team

* Paschalia Kapli
* Sarah Lutteropp
* Tom&aacute;&scaron; Flouri
