#include <stdio.h>

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <stdlib.h>

#include "enums.h"
#include "data.c"

void usage(char* prog)
{
  int i;
  printf("Usage: %s mode modeparams...\n", prog);
  printf("\tmodes:\n");
  printf("\tcustom FILE\tload file from webtool\n");
  printf("\n");
  printf("\tpreset  MODE COLOR BRIGHTNESS SPEED\n");
  printf("\n\t modes: 0:%s",modes[0]);
  for(i=1;i<sizeof(modes)/sizeof(char*);i++) { printf(", %d:%s",i,modes[i]); }
  printf("\n");
  printf("\n\t colors: 0:%s",colors[0]);
  for(i=1;i<sizeof(colors)/sizeof(char*);i++) { printf(", %d:%s",i,colors[i]); }
  printf("\n");
  printf("\n\t brightness: 0 (darkest) to 100 (brightest)\n");
  printf("\n\t speed: 0 (fastest) to 10 (slowest)\n");
  
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argv[0]);
    return -1;
  }

  libusb_context *ctx = NULL;

  int r = libusb_init(&ctx);
  int exitcode = 0;
  if (r < 0) {
    printf("libusb_init error %d\n", r);
    return 1;
  }
  libusb_device_handle *handle = NULL;
  handle = libusb_open_device_with_vid_pid(ctx, 0x1044, 0x7a39);
  if (handle == NULL) {
    printf("Failed to open device!\n");
    libusb_exit(ctx);
    return 1;
  }
  if (libusb_set_auto_detach_kernel_driver(handle, 0) < 0) {
    printf("Kernel ctrl driver auto detach failed.\n");
    goto exit;
  }
  if (libusb_set_auto_detach_kernel_driver(handle, 3) < 0) {
    printf("Kernel driver auto detach failed.\n");
    goto exit;
  }

  r = libusb_claim_interface(handle, 0);
  if (r < 0) {
    printf("Failed to claim ctrl interface! %d\n", r);
    exitcode = 4;
    goto exit;
  }
  r = libusb_claim_interface(handle, 3);
  if (r < 0) {
    printf("Failed to claim interface! %d\n", r);
    exitcode = 2;
    goto exit;
  }

  char* mode = argv[1];
  if (strcmp(mode, "custom") == 0) {
    if (argc < 3) {
      printf("Usage: %s custom file\n", argv[0]);
      exitcode = -1;
      goto exit;
    }
    // Custom mode
    FILE *fd = fopen(argv[2], "rb");
    if (!fd) {
      printf("fopen(%s) failed: %s\n", argv[2], strerror(errno));
      exitcode = -1;
      goto exit;
    }
    fread(m_white_data, 512, 1, fd);
    fclose(fd);

    printf("Setting custom mode\n");
    r = set_custom_mode(handle, m_white_data);
    if (r < 0) {
      printf("Failed to set custom mode!\n");
      exitcode = -1;
      goto exit;
    }
    exitcode = 0;
    goto exit;
  }
  else if (strcmp(mode, "preset") == 0) {
    if (argc < 6) {
      usage(argv[0]);
      exitcode = -1;
      goto exit;
    }
    uint8_t mode=atoi(argv[2]);
    uint8_t color=atoi(argv[3]);
    uint8_t brightness=atoi(argv[4]);
    uint8_t speed=atoi(argv[5]);

    if( mode >= sizeof(modes)/sizeof(char*) )
      {
	printf("incorrect mode value: %d\n",mode);
	exitcode = -1;
	goto exit;
      }
    if( color >= sizeof(colors)/sizeof(char*) )
      {
	printf("incorrect color value: %d\n",color);
	exitcode = -1;
	goto exit;
      }
    if( brightness > 100)
      {
	printf("incorrect brightness value: %d\n",brightness);
	exitcode = -1;
	goto exit;
      }
    if( speed > 10)
      {
	printf("incorrect speed value: %d\n",speed);
	exitcode = -1;
	goto exit;
      }
    
    printf("Setting preset mode %d:%s %d:%s brightenss:%d speed:%d\n",
	   mode,modes[mode],color,colors[color],brightness,speed);
    r = set_mode(handle, mode,color,brightness,speed);
    if (r < 0) {
      printf("Failed to set custom mode!\n");
      exitcode = -1;
      goto exit;
    }
  }
  else {
    usage(argv[0]);
  }
  

exit:
  libusb_release_interface(handle, 0);
  libusb_release_interface(handle, 3);
  libusb_close(handle);
  libusb_exit(ctx);
  return exitcode;
}
