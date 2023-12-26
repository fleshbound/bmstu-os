/*
* filename: bakery.x
* function: Define constants, non-standard data types and the calling
* 			process in remote calls
*/

struct BAKERY
{
	int num;
	int pid;
	int res;
};

program BAKERY_PROG
{
	version BAKERY_VER
	{
		struct BAKERY GETN(struct BAKERY) = 1;
		struct BAKERY WAIT(struct BAKERY) = 2;
		struct BAKERY PROC(struct BAKERY) = 3;
	} = 1; /* Version number = 1 */
} = 0x20000001; /* RPC program number */
