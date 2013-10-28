#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void s_skip_line(FILE *fp)
{ int ch;
  do
  { ch=getc(fp);
  } while (ch!=EOF && ch!='\n');
  ungetc(ch,fp);
}

int s_skipblanks(FILE *fp)
{ int i=0, ch;
  do
  { ch=getc(fp);
    i++; 
  } while (isspace(ch));
  ungetc(ch,fp);
  return ch;
}


int s_search_in_file_ncs(FILE *f, char *s)
/* Not case sensitive */
{ int l=0, ch;
  int is, ilen, found=0;

  ilen=strlen(s);
  do
  { ch=getc(f);
    is=0;
    while (tolower(ch)==tolower(s[is]))
    { ch=getc(f);
      if (ch==EOF) break;
      is++;
      if (is==ilen)
      { found=1;
        break;
      }
    }
  } while (!found && ch!=EOF);
  ungetc(ch,f);
  return found;
}


int s_search_in_file(FILE *f, char *s)
{ int l=0, ch;
  int is, ilen, found=0;

  ilen=strlen(s);
  do
  { ch=getc(f);
    is=0;
    while (ch==s[is])
    { ch=getc(f);
      if (ch==EOF) break;
      is++;
      if (is==ilen)
      { found=1;
        break;
      }
    }
  } while (!found && ch!=EOF);
  ungetc(ch,f);
  return found;
}
