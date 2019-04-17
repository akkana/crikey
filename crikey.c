/*
 * Key Event simulation program.
 * Plug in to your window manager's key handler,
 * to allow for binding passwords, etc. to a key.
 *
 * Copyright 2003 by Akkana Peck, http://www.shallowsky.com/software/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <X11/Intrinsic.h> // for TRUE
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <stdlib.h>    // for atoi
#include <unistd.h>    // for sleep

static int Debug = 0;

/* Should we use the XTest extension, or just XSendEvent? */
static int UseXTest = 0;

struct {
    char ch;
    char* keySymName;
} NonPrintables[] =
{
    { ' ', "space" },
    { '\t', "Tab" },
    { '\n', "Return" },  // for some reason this needs to be cr, not lf
    { '\r', "Return" },
    { '\e', "Escape" },
    { '!', "exclam" },
    { '#', "numbersign" },
    { '$', "dollar" },
    { '&', "ampersand" },
    { '\'', "apostrophe" },
    { '(', "parenleft" },
    { ')', "parenright" },
    { '*', "asterisk" },
    { '=', "equal" },
    { '+', "plus" },
    { ',', "comma" },
    { '-', "minus" },
    { '.', "period" },
    { '/', "slash" },
    { ':', "colon" },
    { ';', "semicolon" },
    { '<', "less" },
    { '>', "greater" },
    { '?', "question" },
    { '@', "at" },
    { '[', "bracketleft" },
    { ']', "bracketright" },
    { '\\', "backslash" },
    { '^', "asciicircum" },
    { '_', "underscore" },
    { '`', "grave" },
    { '{', "braceleft" },
    { '|', "bar" },
    { '}', "braceright" },
    { '~', "asciitilde" },
};

static void simulateKeyPress(Display *disp, char *keyname)
{
    KeyCode keycode = 0;
    KeySym keysym = XStringToKeysym(keyname);
    if (keysym == 0) {
        int i;
        for (i=0; i < ((sizeof NonPrintables) / (sizeof *NonPrintables)); ++i)
            if (*keyname == NonPrintables[i].ch) {
                keysym = XStringToKeysym(NonPrintables[i].keySymName);
                if (Debug)
                    printf("Found a nonprintable: name = %s, keysym = %ld\n",
                           NonPrintables[i].keySymName, keysym);
                break;
            }
    }
    if (keysym == 0) {
        printf("Can't simulate key '%c'\n", *keyname);
        return;
    }
    if (Debug)
        printf("Key is '%c', keysym is %ld\n", *keyname, keysym);
    keycode = XKeysymToKeycode(disp, keysym);
    if (UseXTest) {
        XTestGrabControl(disp, True);
        if (Debug)
            printf("Calling XTestFakeKeyEvent(%p, %d, 1, 0)\n", disp, keycode);
        XTestFakeKeyEvent(disp, keycode, True, 0);
    }
    else {
        XKeyEvent kevent;
        Window focuswin;
        int revert_to;
        XGetInputFocus(disp, &focuswin, &revert_to);
        if (focuswin == 0) {
            printf("No focused window!\n");
            return;
        }
        kevent.display = disp;
        kevent.window = focuswin;
        kevent.root = DefaultRootWindow(disp);
        kevent.subwindow = None;
        kevent.time = CurrentTime;
        kevent.x = 1;
        kevent.y = 1;
        kevent.x_root = 1;
        kevent.y_root = 1;
        kevent.same_screen = TRUE;
        kevent.type = KeyPress;
        kevent.keycode = keycode;
        kevent.state = 0;    /* or get current modifiers? */

        XSendEvent(disp, focuswin, TRUE, KeyPressMask, (XEvent *)&kevent);
    }
    XSync(disp, False);
    XTestGrabControl(disp, False);
}

static void simulateKeyPressForString(Display* disp, char* s)
{
    char buf[2];
    while (*s) {
        if (*s == '\\' && *(s+1) != '\0') {
            switch (*(++s))
            {
              case 'n':
                  buf[0] = '\n';
                  break;
              case 'r':
                  buf[0] = '\r';
                  break;
              case 't':
                  buf[0] = '\t';
                  break;
              default:
                  --s;
                  buf[0] = '\\';
                  break;
            }
        }
        else
            buf[0] = *s;
        buf[1] = '\0';
        simulateKeyPress(disp, buf);
        ++s;
    }
}

int main(int argc, char** argv)
{
    int i;
    Display* disp = XOpenDisplay(0);
    int op, ev, er;

    while (argc > 1 && argv[1][0] == '-') {
        switch(argv[1][1]) {
          case 's':  // sleep
              if (argc > 2) {
                  int sleeptime = atoi(argv[2]);
                  if (Debug)
                      printf("Sleeping for %d seconds\n", sleeptime);
                  sleep(sleeptime);
                  --argc;
                  ++argv;
              }
        }
        --argc;
        ++argv;
    }

    /* Decide whether we can use the XTest extension */
    UseXTest = XQueryExtension(disp, "XTEST", &op, &ev, &er);
    if (Debug) {
        if (UseXTest)
            printf("Using XTest Extension\n");
        else
            printf("No XTest Extension: Using XSendEvent\n");
    }

    for (i=1; i < argc; ++i) {
        simulateKeyPressForString(disp, argv[i]);
        if (i < argc-1)
            simulateKeyPress(disp, " ");
    }

    return 0;
}


