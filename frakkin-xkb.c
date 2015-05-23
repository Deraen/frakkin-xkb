#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

int main (int argc, char** argv)
{
  static int help = 0;
  const char* command = NULL;
  int timeout = 0;

  static struct option long_options[] = {
    {"help", no_argument, &help, 1},
    {"timeout", required_argument, 0, 't'}
  };

  while (1) {
    int i = 0;
    int c = getopt_long(argc, argv, "t:", long_options, &i);

    if (c == -1) break;

    switch (c) {
      case 't':
        timeout = strtol(optarg, NULL, 10);
        break;
    }
  }

  if (optind < argc) {
    command = argv[optind];
  }

  if (help > 0) {
    printf("\
Usage: %s [OPTION] COMMAND\n\
Run COMMAND when a input device is plugged into the system.\n\
\n\
  -t, --timeout          Milliseconds to wait befure running the command\n\
      --help             Display this help and exit\n\
      ", argv[0]);

    exit(0);
  }

  if (command == NULL) {
    printf("Missing command option");
    exit(1);
  }

  struct udev *udev;
  struct udev_device *dev;

  struct udev_monitor *mon;
  int fd;

  udev = udev_new();
  if (!udev) {
    printf("Can't create udev\n");
    exit(1);
  }

  mon = udev_monitor_new_from_netlink(udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
  udev_monitor_enable_receiving(mon);
  fd = udev_monitor_get_fd(mon);

  while (1) {
    fd_set fds;
    int ret;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // Last param is timeout, NULL = forever
    ret = select(fd+1, &fds, NULL, NULL, NULL);

    if (ret > 0 && FD_ISSET(fd, &fds)) {
      dev = udev_monitor_receive_device(mon);
      if (dev) {
        const char* act = udev_device_get_action(dev);
        const char* key = udev_device_get_property_value(dev, "ID_INPUT_KEY");
        // Two events are generated, other has node value set and other doesn't
        const char* node = udev_device_get_devnode(dev);

        if ((strncmp(act, "add", 3) == 0 || strncmp(act, "modify", 6) == 0)
            && strncmp(key, "1", 1) == 0
            && node == NULL)
        {
          if (timeout > 0) {
            usleep(timeout * 1000);
          }
          system(command);
        }
        udev_device_unref(dev);
      }
      else {
        printf("No Device from receive_device(). An error occured.\n");
      }
    }
  }

  udev_unref(udev);

  return 0;
}
