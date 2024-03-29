/*
 *   Copyright (C) 2007 by David Zoltan Kedves
 *   kedazo@gmail.com
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define _POSIX_C_SOURCE 2

#include <stdio.h> //Standard headers
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libxml/xmlmemory.h> //libxml2 headers
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/threads.h>
#include <pthread.h> //POSIX threads

#include <fcntl.h>
#include <sys/wait.h>
#include <linux/wait.h>
#include <signal.h>


char default_ABC[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char CMD_DETECT[] = "file --mime-type -b %s"; // this command returns mime type for file

const char* TYPE[] = {"rar", "7z", "zip", NULL}; // the last "" signals the end of the list
const char* MIME[] = {"application/x-rar", "application/octet-stream", "application/x-zip", NULL};
const char* CMD[] = {"unrar t -y -p%s %s 2>&1", "7z t -y -p%s %s 2>&1", "unzip -P%s -t %s 2>&1", NULL};

#define PWD_LEN 100
