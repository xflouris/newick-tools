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

char ** parse_labels(const char * filename, int * count)
{
  FILE * fp;
  const size_t alloc_step = 100;
  size_t alloc_max = 100;
  unsigned int tip_count = 0;

  char line[4096];

  char ** tip_list;

  tip_list = (char **)xmalloc(alloc_max*sizeof(char*));

  fp = fopen(filename, "r");
  if (!fp)
    fatal("Cannot open file %s\n", filename);

  if (!opt_quiet)
    fprintf(stdout, "Reading list of labels\n");

  while (fgets(line,1024,fp))
  {
    if (alloc_max == tip_count)
    {
      alloc_max += alloc_step;
      char ** temp = (char **)xmalloc(alloc_max*sizeof(char*));
      memcpy(temp,tip_list, tip_count*sizeof(char *));
      free(tip_list);
      tip_list=temp;
    }

    long len;
    if (strchr(line,'\r'))
      len = xstrchrnul(line,'\r') - line;
    else
      len = xstrchrnul(line,'\n') - line;

    line[len] = 0;

    tip_list[tip_count] = xstrdup(line);

    if (!opt_quiet)
      printf("%d: %s\n", tip_count+1, line);
    ++tip_count;
  }
  if (!opt_quiet)
    printf("\n");

  fclose(fp);

  *count = tip_count;

  return tip_list;
}
