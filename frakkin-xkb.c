#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/XInput2.h>

void reset_timer(struct timeval** tv)
{
  if (*tv) free(*tv);
  *tv = (struct timeval*) malloc(sizeof(struct timeval));
  (*tv)->tv_sec = 0;
  (*tv)->tv_usec = 250;
}

int main (int argc, char** argv)
{
  static int help = 0;
  const char* command = NULL;

  static struct option long_options[] = {
    {"help", no_argument, &help, 1},
  };

  while (1)
  {
    int i = 0;
    int c = getopt_long(argc, argv, "t:", long_options, &i);

    if (c == -1) break;

    switch (c) {
    }
  }

  if (optind < argc)
  {
    command = argv[optind];
  }

  if (help > 0)
  {
    printf("\
Usage: %s [OPTION] COMMAND\n\
Run COMMAND when a input device is plugged into the system.\n\
\n\
      --help             Display this help and exit\n\
      ", argv[0]);

    exit(0);
  }

  if (command == NULL)
  {
    printf("Missing command option");
    exit(1);
  }

  Display* display = XOpenDisplay(NULL);

  int xi_opcode;
  int event;
  int error;
  if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
    printf("X Input extension not available\n");
    exit(1);
  }

  unsigned char mask[2] = { 0 };
  XISetMask(mask, XI_HierarchyChanged);

  XIEventMask event_mask;
  event_mask.deviceid = XIAllDevices;
  event_mask.mask = mask;
  event_mask.mask_len = sizeof(mask);

  Window window = DefaultRootWindow(display);

  XISelectEvents(display, window, &event_mask, 1);

  fd_set in_fds;
  int x11_fd = ConnectionNumber(display);
  struct timeval *tv = NULL;

  // Small timeout to filter out multiple events triggered by the same change
  while (1)
  {
    FD_ZERO(&in_fds);
    FD_SET(x11_fd, &in_fds);

    if (XPending(display) || select(x11_fd+1, &in_fds, 0, 0, tv))
    {
      XEvent ev;
      XGenericEventCookie *cookie = &ev.xcookie;
      XNextEvent(display, &ev);

      if (XGetEventData(display, cookie)
          && cookie->type == GenericEvent
          && cookie->extension == xi_opcode)
      {
        XIDeviceEvent *event = (XIDeviceEvent*) cookie->data;

        switch (event->evtype)
        {
          case XI_HierarchyChanged:
            reset_timer(&tv);
            break;
        }
      }

      XFreeEventData(display, cookie);
    }
    else
    {
      system(command);
      if (tv) free(tv);
      tv = NULL;
    }
  }

  return 0;
}
