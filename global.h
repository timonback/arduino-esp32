#ifndef GLOBAL_H
#define GLOBAL_H

#define NULL ((void *)0)

#define FEATURE_FS
#ifdef FEATURE_FS
#include "lib_fs.h"
#endif

#define FEATURE_WIFI
#ifdef FEATURE_WIFI
#include "lib_wifi.h"
#endif

#include "lib_display.h"
#include "vm/vm.h"

VM vm;

#endif