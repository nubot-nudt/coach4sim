/* AUTOGEN File: rtdb_sizeof_tmp.cpp */

#include <stdio.h>
#include "teammatesinfo.h"

#include "common.h"
using namespace nubot;
int main(void)
{
	FILE* f;
	f= fopen("rtdb_size.tmp", "w");
	fprintf(f, "%u\n", sizeof(Teammatesinfo));
	fclose(f);

	return 0;
}

/* EOF: rtdb_sizeof_tmp.cpp */
