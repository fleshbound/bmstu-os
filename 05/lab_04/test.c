#include <stdio.h>
#include <stdlib.h>

int main()
{
	char *arr = calloc(sizeof(char), 10);
	char **fp = &(*arr);
	char **yp = &(*(arr + sizeof(char *)));
	printf("arr=%p\nfp =%p\n*fp=%p\n yp=%p\n", arr, fp, *fp, yp);

	return 0;
}
