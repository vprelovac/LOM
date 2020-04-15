
/* $Id: mapwld.c,v 1.1 1995/06/01 22:07:12 neil Exp $ */

/*
 * $Log: mapwld.c,v $
 * Revision 1.1  1995/06/01  22:07:12  neil
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define UP 4
#define DOWN 5

/* #define DEBUG /*  debug info  */

int nrooms, vmax, vmin;
int minx, miny, minz, maxx, maxy, maxz;

typedef struct
  {
    int vnum, x, y, z, flag;
    int dirs[6];
  }
roomdata;

roomdata *rooms;

int *trans_table;

/*
 *  anum -- finds the real array address of the virtual address vnum
 */

int
anum (int v)
{
  int i, i_low, i_high;

  if (v < vmin || v > vmax)
    return -1;

  i = nrooms / 2;
  i_low = 0;
  i_high = nrooms - 1;
  while (trans_table[i] != v)
    {
      if (v > trans_table[i])
	{
	  i_low = i;
	  i = (i_low + i_high + 1) / 2;
	}
      else if (v < trans_table[i])
	{
	  i_high = i;
	  i = (i_low + i_high) / 2;
	}
      if (i_high - i_low <= 1)
	{
	  if (trans_table[i] != v && trans_table[i_low] != v
	      && trans_table[i_high] != v)
	    return -1;
	}
    }
  return i;
}

/*
 *  print_info -- prints info about an unattached exit
 */

void
print_info (int oldvnum, int dir, int vnum)
{
  if (oldvnum < 0 || vnum < 0)
    return;

  printf ("Out of zone:  %d ", oldvnum);
  switch (dir)
    {
    case NORTH:
      printf ("north to room %d\n", vnum);
      break;
    case EAST:
      printf ("east to room %d\n", vnum);
      break;
    case SOUTH:
      printf ("south to room %d\n", vnum);
      break;
    case WEST:
      printf ("west to room %d\n", vnum);
      break;
    case UP:
      printf ("up to room %d\n", vnum);
      break;
    case DOWN:
      printf ("down to room %d\n", vnum);
      break;
    }
}

/*
 *  check_dir -- tries to establish the coordinates of rooms surrounding
 *               a given room and returns a one if the coordinates are
 *               inconsistent
 */

int
check_dir (int trnum, int x, int y, int z)
{

  int return_val;

#ifdef DEBUG
  fprintf (stderr, ".");
#endif /* DEBUG */

  if (rooms[trnum].flag)
    {
      if (x != rooms[trnum].x || y != rooms[trnum].y || z != rooms[trnum].z)
	{
	  fprintf (stderr, "Inconsistent map, room #%d.\n\r", rooms[trnum].vnum);
	  return 1;
	}
      return 0;
    }

  rooms[trnum].flag = 1;
  rooms[trnum].x = x;
  rooms[trnum].y = y;
  rooms[trnum].z = z;

  minx = (x < minx) ? x : minx;
  maxx = (x > maxx) ? x : maxx;
  miny = (y < miny) ? y : miny;
  maxy = (y > maxy) ? y : maxy;
  minz = (z < minz) ? z : minz;
  maxz = (z > maxz) ? z : maxz;

  return_val = 0;

  if (anum (rooms[trnum].dirs[NORTH]) >= 0)
    if (check_dir (anum (rooms[trnum].dirs[NORTH]), x, y + 1, z))
      return_val++;
  if (anum (rooms[trnum].dirs[EAST]) >= 0)
    if (check_dir (anum (rooms[trnum].dirs[EAST]), x + 1, y, z))
      return_val++;
  if (anum (rooms[trnum].dirs[SOUTH]) >= 0)
    if (check_dir (anum (rooms[trnum].dirs[SOUTH]), x, y - 1, z))
      return_val++;
  if (anum (rooms[trnum].dirs[WEST]) >= 0)
    if (check_dir (anum (rooms[trnum].dirs[WEST]), x - 1, y, z))
      return_val++;
  if (anum (rooms[trnum].dirs[UP]) >= 0)
    if (check_dir (anum (rooms[trnum].dirs[UP]), x, y, z + 1))
      return_val++;
  if (anum (rooms[trnum].dirs[DOWN]) >= 0)
    if (check_dir (anum (rooms[trnum].dirs[DOWN]), x, y, z - 1))
      return_val++;

  return return_val;
}

/*
 *  readworld -- reads in a world file into the array rooms and sets
 *               up the virtual to real translation table
 */

int
readworld (char *filename)
{
  FILE *infile;
  int i, size_rooms, vnum, dirs, dest, count, dflags, key, dir_flag, done_flag;
  char s[256];

  size_rooms = 10;
  i = -1;
  nrooms = 0;
  dir_flag = 0;
  done_flag = 0;
  vmin = vmax = -1;
  rooms = calloc (size_rooms, sizeof (roomdata));
  trans_table = (int *) calloc (size_rooms, sizeof (int));
  if ((infile = fopen (filename, "r")) == NULL)
    return 1;

#ifdef DEBUG
  fprintf (stderr, "Reading %s.", filename);
#endif /* DEBUG */

  while (!done_flag)
    {
      fgets (s, sizeof (s), infile);
      if ((sscanf (s, "#%d\n", &vnum)) == 1)
	{
#ifdef DEBUG
	  fprintf (stderr, ".");
#endif /* DEBUG */
	  i++;
	  if (size_rooms - i < 3)
	    {
	      size_rooms += 5;
	      rooms = (roomdata *) realloc (rooms, sizeof (roomdata) * size_rooms);
	      trans_table = (int *) realloc (trans_table, sizeof (int) * size_rooms);
	    }
	  rooms[i].vnum = vnum;
	  trans_table[i] = vnum;
	  for (dirs = 0; dirs < 6; dirs++)
	    rooms[i].dirs[dirs] = -1;
	}
      else if ((sscanf (s, "D%d\n", &dirs)) == 1)
	{
	  dir_flag = 1;
	  count = 0;
	}
      else if (dir_flag)
	{
	  if (strstr (s, "~"))
	    count++;
	  else if (count == 2)
	    {
	      dir_flag = 0;
	      sscanf (s, "%d %d %d", &dflags, &key, &dest);
	      rooms[i].dirs[dirs] = dest;
	    }
	}
      else if (!strncmp ("$", s, 1))
	done_flag = 1;
    }

  nrooms = i;

  vmin = vmax = rooms[0].vnum;
  for (i = 1; i < nrooms; i++)
    {
      vmin = (vmin > rooms[i].vnum) ? rooms[i].vnum : vmin;
      vmax = (vmax < rooms[i].vnum) ? rooms[i].vnum : vmax;
    }

#ifdef DEBUG
  fprintf (stderr, "\n");
#endif /* DEBUG */
  return 0;
}

/*
 *   writetable -- write out the adjancy matrix
 */

void
writetable (char *filename, int oflg)
{
  int i, j;
  FILE *outfile;

  if (oflg)
    {
      /*      12345678 12345678 12345678 12345678 12345678 12345678 1235468 */
      printf ("**Room**    North     East    South     West       Up    Down\n");
    }
  else
    {
      outfile = fopen (filename, "w");
      fprintf (outfile, "**Room**    North     East    South     West       Up    Down\n");
    }

  for (i = 0; i < nrooms; i++)
    {
      if (oflg)
	printf ("%8d %8d %8d %8d %8d %8d %8d\n", rooms[i].vnum,
		rooms[i].dirs[NORTH], rooms[i].dirs[EAST],
		rooms[i].dirs[SOUTH], rooms[i].dirs[WEST],
		rooms[i].dirs[UP], rooms[i].dirs[DOWN]);
      else
	fprintf (outfile, "%8d %8d %8d %8d %8d %8d %8d\n", rooms[i].vnum,
		 rooms[i].dirs[NORTH], rooms[i].dirs[EAST],
		 rooms[i].dirs[SOUTH], rooms[i].dirs[WEST],
		 rooms[i].dirs[UP], rooms[i].dirs[DOWN]);
    }
}

/*
 *   writecoordinates -- write out the coordinate list
 */

void
writecoordinates (void)
{
  int i;

  printf ("**Room**   **X**   **Y**  **Z**\n");
  for (i = 0; i < nrooms; i++)
    printf ("%8d   %5d   %5d   %5d\n", rooms[i].vnum, rooms[i].x, rooms[i].y,
	    rooms[i].z);

  printf ("\n\n");
}

/*
 *   findcoords -- find the vnum of a room with particular coords
 */

int
findcoords (int x, int y, int z)
{
  int i;

  for (i = 0; i < nrooms; i++)
    if (rooms[i].x == x && rooms[i].y == y && rooms[i].z == z)
      return rooms[i].vnum;

  return -1;
}

/*
 *   writemap -- write out the map
 */

void
writemap (char *filename, int oflg)
{
  int i, j, k, level, x, y, rnum, foundline;
  char line1[1000], line2[1000], line3[1000], numstr[10];
  FILE *outfile;

  if (!oflg)
    outfile = fopen (filename, "w");

  for (level = minz; level <= maxz; level++)
    {
      if (oflg)
	printf ("Level %d:\n\n", level);
      else
	fprintf (outfile, "Level %d:\n\n", level);
      for (y = maxy; y >= miny; y--)
	{
	  strcpy (line1, "");
	  strcpy (line2, "");
	  strcpy (line3, "");
	  foundline = 0;
	  for (x = minx; x <= maxx; x++)
	    if ((rnum = findcoords (x, y, level)) >= 0)
	      {
		foundline++;
		if (rooms[anum (rnum)].dirs[WEST] > 0)
		  if (anum (rooms[anum (rnum)].dirs[WEST]) >= 0)
		    strcat (line2, "-");
		  else
		    strcat (line2, "=");
		else
		  strcat (line2, " ");
		if (rooms[anum (rnum)].dirs[NORTH] > 0)
		  if (anum (rooms[anum (rnum)].dirs[NORTH]) >= 0)
		    strcat (line1, "   |  ");
		  else
		    strcat (line1, "   ^  ");
		else
		  strcat (line1, "      ");
		if (rooms[anum (rnum)].dirs[UP] > 0)
		  if (anum (rooms[anum (rnum)].dirs[UP]) >= 0)
		    strcat (line1, "/");
		  else
		    strcat (line1, "~");
		else
		  strcat (line1, " ");
		if (rooms[anum (rnum)].dirs[DOWN] > 0)
		  if (anum (rooms[anum (rnum)].dirs[DOWN]) >= 0)
		    strcat (line3, "/");
		  else
		    strcat (line3, "~");
		else
		  strcat (line3, " ");
		if (rooms[anum (rnum)].dirs[SOUTH] > 0)
		  if (anum (rooms[anum (rnum)].dirs[SOUTH]) >= 0)
		    strcat (line3, "  |   ");
		  else
		    strcat (line3, "  v   ");
		else
		  strcat (line3, "      ");
		sprintf (numstr, "%5d", rnum);
		for (i = 0; i < 5; i++)
		  if (numstr[i] == ' ')
		    numstr[i] = '*';
		strcat (line2, numstr);
		if (rooms[anum (rnum)].dirs[EAST] > 0)
		  if (anum (rooms[anum (rnum)].dirs[EAST]) >= 0)
		    strcat (line2, "-");
		  else
		    strcat (line2, "=");
		else
		  strcat (line2, " ");
	      }
	    else
	      {
		strcat (line1, "       ");
		strcat (line2, "       ");
		strcat (line3, "       ");
	      }
	  if (foundline)
	    if (oflg)
	      printf ("%s\n%s\n%s\n", line1, line2, line3);
	    else
	      fprintf (outfile, "%s\n%s\n%s\n", line1, line2, line3);
	}
      if (oflg)
	printf ("\n\n");
      else
	fprintf (outfile, "\n\n");
    }

  if (!oflg)
    fclose (outfile);
}

/*
 *   main -- the main program...
 */

int
main (int argc, char *argv[])
{
  char filename[80];
  int i, startz;

  rooms = NULL;
  trans_table = NULL;

  if (argc < 2)
    {
      fprintf (stderr, "usage:  %s worldfile [tablefile [mapfile]]\n", argv[0]);
      return 1;
    }

  if (readworld (argv[1]))
    {
      fprintf (stderr, "Unable to read world file (%s)...\n", argv[1]);
      return 1;
    }

  rooms[0].x = rooms[0].y = rooms[0].z = 0;
  for (i = 0; i < nrooms; i++)
    rooms[i].flag = 0;

#ifdef DEBUG
  fprintf (stderr, "Creating coordinates.");
#endif /* DEBUG */
  startz = -1;
  for (i = 0; i < nrooms; i++)
    if (!rooms[i].flag)
      {
	startz = (startz < 0) ? 0 : maxz + 1;
	check_dir (i, 0, 0, startz);
      }
#ifdef DEBUG
  fprintf (stderr, "\n");
#endif /* DEBUG */

  if (argc < 3)
    writetable (" ", 1);
  else
    writetable (argv[2], 0);

  writecoordinates ();

  if (argc < 4)
    writemap (" ", 1);
  else
    writemap (argv[3], 0);
}
