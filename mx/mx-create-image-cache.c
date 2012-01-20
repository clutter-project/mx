/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * makecache.c: creating a texture cache
 *
 * Copyright 2009 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

struct imgcache_element {
  char  filename[256];
  int   width, height;
  int   posX, posY;
  void *ptr;
};

GList *images;

int totalarea = 0;


int sizes[] = { 0, 256, 384, 512, 640, 768, 896, 1024, 1280, 1536, 1792,  2048, -1};


static gint sort_by_size(gconstpointer a,
                         gconstpointer b)
{
  const struct imgcache_element *A=a, *B=b;
  if (A->height > B->height)
    return -1;
  if (A->height < B->height)
    return 1;
  if (A->width > B->width)
    return -1;
  if (A->width < B->width)
    return 1;
  return 0;

}
static void do_one_file(char *filename)
{
  GdkPixbuf *image;
  struct imgcache_element *element;

  if (strlen(filename) >= 256)
    return;

  image = gdk_pixbuf_new_from_file(filename, NULL);
  if (!image)
    return;

  /* we don't want to do files > 256x256 */

  if (gdk_pixbuf_get_width(image) > 256) {
      gdk_pixbuf_unref(image);
      return;
    }
  if (gdk_pixbuf_get_height(image) > 256) {
      gdk_pixbuf_unref(image);
      return;
    }
  element = malloc(sizeof(struct imgcache_element));
  if (!element) {
      gdk_pixbuf_unref(image);
      return;
    }

  memset(element, 0, sizeof(struct imgcache_element));
  element->width = gdk_pixbuf_get_width(image);
  element->height = gdk_pixbuf_get_height(image);
  element->posX = -1;
  element->posY = -1;
  element->ptr = image;
  totalarea += element->width * element->height;
  strncpy(element->filename, filename, 256);

  if (element->width > sizes[0])
    sizes[0] = element->width;

  images = g_list_append(images, element);
}

static int get_effective_Y(int X1,
                           int X2)
{
  int Y = 0;
  GList *item;
  struct imgcache_element *element;
  item = g_list_first(images);
  while (item) {
      element = item->data;
      item = g_list_next(item);
      if (element->posX == -1)
        continue;
      if (element->posX >= X2)
        continue;
      if (element->posX + element->width <= X1)
        continue;
      /* at this point we know the element overlaps our range */
      if (element->posY + element->height > Y)
        Y = element->posY + element->height;
    }
  return Y;

}

static inline uint32_t get_pixel(struct imgcache_element *element,
                                 int                      X,
                                 int                      Y)
{
  GdkPixbuf *buf;
  unsigned char *p;
  buf = element->ptr;

  p = gdk_pixbuf_get_pixels(buf);
  p += Y * gdk_pixbuf_get_rowstride(buf);
  p += X * 4;

  return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static int can_scooch(struct imgcache_element *one,
                      struct imgcache_element *two)
{
  int i,x,y;

  if (!one)
    return 0;
  if (!two)
    return 0;

  if (one->posY != two->posY)
    return 0;
  if (one->height != two->height)
    return 0;
  if (one->posX + one->width != two->posX)
    return 0;


  i = 0;

  while (i < one->width && i < two->width) {
      i ++;
      for (y = 0; y < one->height; y++) {
          for (x = 0; x < i; x++ ) {
              int p1, p2;

              p1 = get_pixel(one, one->width - i + x,y);
              p2 = get_pixel(two, x, y);
              if ( p1 != p2)
                return i-1;
            }
        }
    }
  return i;
}
static int do_placement(int maxX)
{
  int maxY;
  int X, Y, baseY;
  int tempY, tempX, biggestX = -1;
  struct imgcache_element *element, *element2, *prev = NULL;
  GList *item, *item2;
  int workdone = 1;

  maxY = 0;

  X = 0;
  Y = 0;


  while (workdone) {
      workdone = 0;

      X = 0;
      item = g_list_first(images);
      while (item) {
          int sc;
          element = item->data;
          item = g_list_next(item);

          if (element->posX != -1)               /* already placed */
            continue;

          /* first, check if we fit */
          if (X + element->width > maxX)
            continue;

          tempY = get_effective_Y(X, X + element->width);

          element->posX = X;
          element->posY = tempY;
          workdone ++;


          sc = can_scooch(prev, element);

          element->posX -= sc;

          if (element->posY + element->height > maxY)
            maxY = element->posY + element->height;

          prev = element;


          /* shortcut .. find other icons that fit us without expanding vertically*/

          tempX = X;
          tempY += element->height;
repeat:
          biggestX = -1;
          item2 = item;
          baseY = element->posY + element->height;
          while (item2 && !sc) {
              element2 = item2->data;
              item2 = g_list_next(item2);
              if (element2->posX != -1)
                continue;
              if (tempX + element2->width > X + element->width)
                continue;
              if (element2->height > maxY - baseY)
                continue;

              tempY = get_effective_Y(tempX, tempX+element2->width);

              if (element2->height > maxY - tempY)
                continue;
              element2->posX = tempX;
              element2->posY = tempY;

              if (element2->posX + element2->width > biggestX && element2->width != element->width)
                biggestX = element2->posX + element2->width;
              if (element2->width == element->width)
                biggestX = -1;
              workdone++;
              prev = NULL;
            }
          if (biggestX > 0) {
              tempX = biggestX;
              goto repeat;
            }
          X = element->posX +  element->width;
        }
    }

  X = 0;
  Y = 0;
  /* find the bounding box */
  item = g_list_first(images);
  while (item) {
      element = item->data;
      item = g_list_next(item);
      if (element->posX + element->width > X)
        X = element->posX + element->width;
      if (element->posY + element->height > Y)
        Y = element->posY + element->height;
    }

  /* penalties for getting too big for a texture */
  if (X > 2048)
    X *= 4;
  if (Y > 2048)
    Y *= 4;
  return X * Y;
}

static void clear_placement(void)
{
  struct imgcache_element *element;
  GList *item;

  /* find the bounding box */
  item = g_list_first(images);
  while (item) {
      element = item->data;
      item = g_list_next(item);
      element->posX = -1;
      element->posY = -1;
    }
}

static void optimal_placement(void)
{
  int bestX = -1;
  int best_score = INT_MAX;
  int score;
  int i;
  int minX;
  i = 0;

  /* skip X values that would lead to a too large Y dimension */
  minX = totalarea / 2048;

  if (minX > 1000)
    minX = 0;
  while (sizes[i] > 0) {
      if (sizes[i] < minX) {
          i++;
          continue;
        }
      score = do_placement(sizes[i]);
      if (score < best_score) {
          bestX = sizes[i];
          best_score = score;
        }
      clear_placement();
      i++;
    }

  do_placement(bestX);
  printf("Best score is %i, %0.1f %% waste\n", best_score, (100.0*best_score / totalarea) -100.0);
}

static int make_final_image(char *filename)
{
  int maxX = 0, maxY = 0;
  struct imgcache_element *element;
  GList *item;
  GdkPixbuf *final;

  /* find the bounding box */
  item = g_list_first(images);
  while (item) {
      element = item->data;
      item = g_list_next(item);
      if (element->posX + element->width > maxX)
        maxX = element->posX + element->width;
      if (element->posY + element->height > maxY)
        maxY = element->posY + element->height;
    }

  printf("Final image is %ix%i\n", maxX, maxY);

  if (!maxX)
    return 0;
  if (!maxY)
    return 0;
  final = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, maxX, maxY);
  assert(final != NULL);

  item = g_list_first(images);
  while (item) {
      element = item->data;
      item = g_list_next(item);
      if (element->posX == -1)
        continue;
      gdk_pixbuf_copy_area(element->ptr, 0, 0, element->width, element->height, final, element->posX, element->posY);
    }

  gdk_pixbuf_save(final, filename, "png", NULL, NULL);
  return 1;
}

static void makecache(char *directory,
                      int   recurse)
{
  GDir *dir;
  GError *error = NULL;

  dir = g_dir_open (directory, 0, &error);
  if (!dir) {
      printf("Error opening %s: %s\n", directory, error->message);
      g_clear_error (&error);
      return;
    }

  while (TRUE) {
      const char *name = g_dir_read_name (dir);
      char *fullpath = g_build_filename (directory, name, NULL);

      if (!name)
        break;
      if (name[0] == '.')
        continue;
      if (recurse && g_file_test (fullpath, G_FILE_TEST_IS_DIR)) {
          makecache(fullpath, recurse);
        }

      if (recurse && g_file_test (fullpath, G_FILE_TEST_IS_REGULAR)) {
          do_one_file(fullpath);
        }

      g_free (fullpath);
  }

  g_dir_close (dir);

  images = g_list_sort(images, sort_by_size);
}

static void write_cache_file(char *directory,
                             char *pngfile)
{
  FILE *file;
  char *filename = NULL;
  struct imgcache_element element;
  struct imgcache_element *elm;
  GList *item;

  memset(&element, 0, sizeof(element));

  strcpy(&element.filename[0], pngfile);

  filename = g_strdup_printf("%s/mx.cache", directory);

  file = fopen(filename, "w");
  if (!file) {
      fprintf(stderr, "Cannot write cache file: %s\n", filename);
      g_free (filename);
      return;
    }
  fwrite(&element, 1, sizeof(element), file);

  item = g_list_first(images);
  while (item) {
      elm = item->data;
      item = g_list_next(item);
      elm->ptr = NULL;
      fwrite(elm, 1, sizeof(element), file);
    }
  g_free (filename);
  fclose(file);

}

int main(int    argc,
         char **argv)
{
  char *image_file = NULL;

  if (argc <= 1) {
      printf("Usage:\n\t\tmakecache <directory>\n");
      return EXIT_FAILURE;
    }
  g_type_init();
  makecache(argv[1], 1);
  optimal_placement();
  image_file = g_strdup_printf("/var/cache/mx/%08x.png", g_str_hash(argv[1]));
  if (make_final_image(image_file))
    write_cache_file(argv[1], image_file);
  g_free (image_file);
  return EXIT_SUCCESS;
}
