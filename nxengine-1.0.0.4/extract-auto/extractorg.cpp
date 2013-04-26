#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include "extractfiles.fdh"
#include "../nx_logger.h"

#ifdef __MINGW32__
	#include <direct.h>
	#include <io.h>
#endif

#ifdef _XBOX
#include <xtl.h>
#endif

#define HEADER_LEN		25
#define MAX_FILE_SIZE	32768

// Windows .bmp resources don't include the BMP-file headers
const uint8_t credit_header[] = \
{
	0x42, 0x4D, 0x76, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xA0, 0x00,
	0x00, 0x00, 0xF0, 0x00, 0x00
};

const uint8_t pixel_header[] = \
{
	0x42, 0x4D, 0x76, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xA0, 0x00,
	0x00, 0x00, 0x10, 0x00, 0x00
};

#ifdef _WIN32
static struct
{
	const char *filename;
	uint32_t offset;
	uint32_t length;
	uint32_t crc;
	const uint8_t *header;
}
files[] =
{
	"org\\access.org",      0x09b35c, 1138,  0xd965dddb, NULL,
	"org\\balcony.org",     0x09dbbc, 3082,  0x892345ca, NULL,
	"org\\balrog.org",      0x0b45a0, 5970,  0xb02093b8, NULL,
	"org\\breakdown.org",   0x09f5bc, 2570,  0xf80dd62a, NULL,
	"org\\cemetary.org",    0x09ffc8, 4578,  0x2ce377cc, NULL,
	"org\\charge.org",      0x0d28d4, 2770,  0x10dec9d5, NULL,
	"org\\credits.org",     0x0a7eac, 17898, 0xa9ed4834, NULL,
	"org\\egg.org",         0x0feb20, 19626, 0xb651047e, NULL,
	"org\\eyesofflame.org", 0x0aedc0, 21354, 0x6b5ff989, NULL,
	"org\\fanfale1.org",    0x0ae25c, 914,   0xaefd547b, NULL,
	"org\\fanfale2.org",    0x0ae98c, 1074,  0x3a5170a6, NULL,
	"org\\fanfale3.org",    0x0ae5f0, 922,   0x85813929, NULL,
	"org\\gameover.org",    0x0b412c, 1137,  0x525d58f3, NULL,
	"org\\geothermal.org",  0x0b5cf4, 13466, 0xdb4795ac, NULL,
	"org\\gestation.org",   0x0f83c8, 10458, 0xce2e68c1, NULL,
	"org\\gravity.org",     0x0b9190, 20578, 0x64a9318d, NULL,
	"org\\grasstown.org",   0x1037cc, 23706, 0xa27883b6, NULL,
	"org\\hell.org",        0x0be1f4, 18386, 0x93bbf277, NULL,
	"org\\heroend.org",     0x0f1598, 9722,  0xfc64d0d0, NULL,
	"org\\jenka1.org",      0x0c5e54, 8306,  0xb42d7eaa, NULL,
	"org\\jenka2.org",      0x0c7ec8, 11986, 0xc095cbe1, NULL,
	"org\\labyrinth.org",   0x0dbcb8, 14786, 0x0292cf2c, NULL,
	"org\\lastbattle.org",  0x0cd650, 21122, 0x8888dac9, NULL,
	"org\\lastcave.org",    0x0d33a8, 18122, 0x469b38b9, NULL,
	"org\\meltdown2.org",   0x0df67c, 21074, 0x83d08aed, NULL,
	"org\\oppression.org",  0x0c29c8, 13450, 0x3ce4cdbe, NULL,
	"org\\oside.org",       0x0e725c, 25634, 0x1e33b095, NULL,
	"org\\plant.org",       0x0ed680, 11378, 0x3911e040, NULL,
	"org\\pulse.org",       0x0cad9c, 10418, 0x92ef0330, NULL,
	"org\\quiet.org",       0x0f02f4, 4770,  0x0e95a468, NULL,
	"org\\run.org",         0x0ac498, 7618,  0x65a4bb85, NULL,
	"org\\safety.org",      0x09b7d0, 9194,  0x779e83c2, NULL,
	"org\\scorching.org",   0x0faca4, 15994, 0xd09341e2, NULL,
	"org\\seal.org",        0x09e7c8, 3570,  0x373988ad, NULL,
	"org\\theme.org",       0x0a11ac, 25738, 0xf5ace8b0, NULL,
	"org\\toroko.org",      0x0f3b94, 18482, 0xc202de07, NULL,
	"org\\town.org",        0x0e48d0, 10634, 0x6a6aa627, NULL,
	"org\\tyrant.org",      0x0a7638, 2162,  0xc64dc450, NULL,
	"org\\waterway.org",    0x0d7a74, 16962, 0xb533d72a, NULL,
	"org\\white.org",       0x109468, 23714, 0xcff0fb34, NULL,
	"org\\zombie.org",      0x10f180, 5346,  0xd217cc29, NULL,
	NULL
};
#else
static struct
{
	const char *filename;
	uint32_t offset;
	uint32_t length;
	uint32_t crc;
	const uint8_t *header;
}
files[] =
{
	"org/access.org",      0x09b35c, 1138,  0xd965dddb, NULL,
	"org/balcony.org",     0x09dbbc, 3082,  0x892345ca, NULL,
	"org/balrog.org",      0x0b45a0, 5970,  0xb02093b8, NULL,
	"org/breakdown.org",   0x09f5bc, 2570,  0xf80dd62a, NULL,
	"org/cemetary.org",    0x09ffc8, 4578,  0x2ce377cc, NULL,
	"org/charge.org",      0x0d28d4, 2770,  0x10dec9d5, NULL,
	"org/credits.org",     0x0a7eac, 17898, 0xa9ed4834, NULL,
	"org/egg.org",         0x0feb20, 19626, 0xb651047e, NULL,
	"org/eyesofflame.org", 0x0aedc0, 21354, 0x6b5ff989, NULL,
	"org/fanfale1.org",    0x0ae25c, 914,   0xaefd547b, NULL,
	"org/fanfale2.org",    0x0ae98c, 1074,  0x3a5170a6, NULL,
	"org/fanfale3.org",    0x0ae5f0, 922,   0x85813929, NULL,
	"org/gameover.org",    0x0b412c, 1137,  0x525d58f3, NULL,
	"org/geothermal.org",  0x0b5cf4, 13466, 0xdb4795ac, NULL,
	"org/gestation.org",   0x0f83c8, 10458, 0xce2e68c1, NULL,
	"org/gravity.org",     0x0b9190, 20578, 0x64a9318d, NULL,
	"org/grasstown.org",   0x1037cc, 23706, 0xa27883b6, NULL,
	"org/hell.org",        0x0be1f4, 18386, 0x93bbf277, NULL,
	"org/heroend.org",     0x0f1598, 9722,  0xfc64d0d0, NULL,
	"org/jenka1.org",      0x0c5e54, 8306,  0xb42d7eaa, NULL,
	"org/jenka2.org",      0x0c7ec8, 11986, 0xc095cbe1, NULL,
	"org/labyrinth.org",   0x0dbcb8, 14786, 0x0292cf2c, NULL,
	"org/lastbattle.org",  0x0cd650, 21122, 0x8888dac9, NULL,
	"org/lastcave.org",    0x0d33a8, 18122, 0x469b38b9, NULL,
	"org/meltdown2.org",   0x0df67c, 21074, 0x83d08aed, NULL,
	"org/oppression.org",  0x0c29c8, 13450, 0x3ce4cdbe, NULL,
	"org/oside.org",       0x0e725c, 25634, 0x1e33b095, NULL,
	"org/plant.org",       0x0ed680, 11378, 0x3911e040, NULL,
	"org/pulse.org",       0x0cad9c, 10418, 0x92ef0330, NULL,
	"org/quiet.org",       0x0f02f4, 4770,  0x0e95a468, NULL,
	"org/run.org",         0x0ac498, 7618,  0x65a4bb85, NULL,
	"org/safety.org",      0x09b7d0, 9194,  0x779e83c2, NULL,
	"org/scorching.org",   0x0faca4, 15994, 0xd09341e2, NULL,
	"org/seal.org",        0x09e7c8, 3570,  0x373988ad, NULL,
	"org/theme.org",       0x0a11ac, 25738, 0xf5ace8b0, NULL,
	"org/toroko.org",      0x0f3b94, 18482, 0xc202de07, NULL,
	"org/town.org",        0x0e48d0, 10634, 0x6a6aa627, NULL,
	"org/tyrant.org",      0x0a7638, 2162,  0xc64dc450, NULL,
	"org/waterway.org",    0x0d7a74, 16962, 0xb533d72a, NULL,
	"org/white.org",       0x109468, 23714, 0xcff0fb34, NULL,
	"org/zombie.org",      0x10f180, 5346,  0xd217cc29, NULL,
	NULL
};
#endif

bool extract_org(FILE *exefp)
{
   uint8_t *buffer;
   uint8_t *file;
   uint32_t length;
   uint32_t crc;
   bool check_crc = true;
   bool first_crc_failure = true;

   buffer = (uint8_t *)malloc(MAX_FILE_SIZE);
   crc_init();

   for(int i=0;;i++)
   {
      if (!files[i].filename) break;
      char outfilename[1024];
      retro_create_path_string(outfilename, sizeof(outfilename), g_dir, files[i].filename);

      NX_LOG("[ %s ]\n", outfilename);

      // initialize header if any
      file = buffer;
      length = files[i].length;

      if (files[i].header)
      {
         memcpy(buffer, files[i].header, HEADER_LEN);
         file += HEADER_LEN;
         length += HEADER_LEN;
      }

      // read data from exe
      fseek(exefp, files[i].offset, SEEK_SET);
      fread(file, files[i].length, 1, exefp);

      if (check_crc)
      {
         crc = crc_calc(file, files[i].length);
         if (crc != files[i].crc)
         {
            NX_ERR("File '%s' failed CRC check.\n", outfilename);
            first_crc_failure = false;
         }
      }

      fprintf(stderr, "file: %s\n", files[i].filename);

      // write out the file
      createdir(outfilename);

      FILE *fp = fopen(outfilename, "wb");
      if (!fp)
      {
         NX_ERR("Failed to open '%s' for writing.\n", outfilename);
         free(buffer);
         return 1;
      }

      fwrite(buffer, length, 1, fp);
      fclose(fp);
   }

   free(buffer);
   return 0;
}


static void createdir(const char *fname)
{
	char *dir = strdup(fname);
#if defined(_WIN32)
	char *ptr = strrchr(dir, '\\');
#else
	char *ptr = strrchr(dir, '/');
#endif
	if (ptr)
	{
		*ptr = 0;
		
      #if defined(_XBOX)
      CreateDirectory(dir, NULL);
		#elif defined(_WIN32)
			_mkdir(dir);
		#else
			mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		#endif
	}
	
	free(dir);
}