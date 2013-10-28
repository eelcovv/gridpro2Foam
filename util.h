#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
void s_skip_line(FILE *fp);
int s_skipblanks(FILE *fp);
int s_search_in_file_ncs(FILE *f, char *s);
int s_search_in_file(FILE *f, char *s);

#ifdef __cplusplus
}
#endif

