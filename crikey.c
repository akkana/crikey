/*
 * Key Event simulation program.
 * Plug in to your window manager's key handler,
 * to allow for binding passwords, etc. to a key.
 *
 * Copyright 2003 by Akkana Peck, http://www.shallowsky.com/software/
 * Other contributors:
 *    Efraim Feinstein, 2004
 *    Glen Smith, 2008
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
#include <ctype.h>     // for isdigit

#define VERSION "0.7"

static int Debug = 0;
static int UseXTest = 0;
static int UseStdin = 0;

/* size of the buffer for reading from stdin */
#define BUFSIZE 256

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
    { '%', "percent" },
    { '$', "dollar" },
    { '&', "ampersand" },
    { '"', "quotedbl" },
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

static int isshift(char *keyname) 
{
    char c = keyname[0];
    if (isupper(c)) return 1;
    
    switch(c) {
        case '~':
        case '!':
        case '@':
        case '#':
        case '$':
        case '%':
        case '^':
        case '&':
        case '*':
        case '(':
        case ')':
        case '_':
        case '+':
        case '|':
        case '{':
        case '}':
        case ':':
        case '"':
        case '<':
        case '>':
        case '?':
            return 1;
                       
    }
    return 0;
}

static void simulateKeyPress(Display *disp, char *keyname)
{
    KeyCode keycode = 0;
    KeySym keysym;
    short isShifted;
    KeyCode shiftKeycode = 0;

    keysym = XStringToKeysym(keyname);
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
        printf("crikey: Can't simulate key '%c'\n", *keyname);
        return;
    }

    /* For some reason, '<' needs special treatment:
     * XStringToKeysym("less") returns 60, and
     * XKeysymToKeycode(60) returns keycode 94
     * but generating an event with keycode 94 will send a '>'
     * The keysym needs to be 44 to get a keycode of 59 to send a '<'.
     */
    if (*keyname == '<')
        keycode = 59;
    else
        keycode = XKeysymToKeycode(disp, keysym);
    if (Debug)
        printf("Key is '%c', keysym is %ld, keycode is %d\n",
               *keyname, keysym, keycode);
    isShifted = isshift(keyname);
    if (isShifted && shiftKeycode == 0) {
        shiftKeycode = XKeysymToKeycode(disp, XK_Shift_L);
        printf("Keycode for shift is %d\n", shiftKeycode);
    }

    if (UseXTest) {
        XTestGrabControl(disp, True);
        if (Debug)
            printf("Calling XTestFakeKeyEvent(%p, %d, 1, 0)\n", disp, keycode);
        if (isShifted)
            XTestFakeKeyEvent(disp, shiftKeycode, True, 0);  /* shift press */
        XTestFakeKeyEvent(disp, keycode, True, 0);      /* key press */
        XTestFakeKeyEvent(disp, keycode, False, 0);     /* key release */
        if (isShifted)
            XTestFakeKeyEvent(disp, shiftKeycode, False, 0);  /* shift rel */
        XSync(disp, False);
        XTestGrabControl(disp, False);
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
        kevent.state = isShifted ? ShiftMask : 0;
                                       /* or get current modifiers? */
        if (Debug)
            printf("Sending an event with keycode = %d, shift mask %d\n",
                   kevent.keycode, kevent.state);

        XSendEvent(disp, focuswin, TRUE, KeyPressMask, (XEvent *)&kevent);
        /* Wonder if we might ever need the key release --
         * but in some contexts, that actually gets interpreted
         * as another key press!
        XSendEvent(disp, focuswin, TRUE, KeyReleaseMask, (XEvent *)&kevent);
         */
        XSync(disp, False);
    }
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

    printf("<> are %ld, %ld\n\n",
           XStringToKeysym("less"), XStringToKeysym("greater"));
    while (argc > 1 && argv[1][0] == '-') {
        switch(argv[1][1]) {
          case 'd': // debug mode
              Debug=1;
              break;      
          case 's':  // sleep
              if (isdigit(argv[1][2])) {
                  int sleeptime = atoi(argv[1]+2);
                  printf("atoi(%s+n)\n", argv[1]);
                  if (Debug)
                      printf("Sleeping for %d seconds\n", sleeptime);
                  sleep(sleeptime);
              }
              else if (argc > 2 && isdigit(argv[2][0])) {
                  int sleeptime = atoi(argv[2]);
                  if (Debug)
                      printf("Sleeping for %d seconds\n", sleeptime);
                  sleep(sleeptime);
                  --argc;
                  ++argv;
              }
              break;

          case 't':  // use X Test extension if available.  Default: don't.
              UseXTest = 1;
              break;
          case 'i':  // take string from standard input
              UseStdin = 1;
              break;
          default:
              printf("crikey! version %s\n", VERSION);
              printf("Usage: crikey [-t] [-s sleeptime] [-i] string...\n");
              exit(1);
        }
        --argc;
        ++argv;
    }

    /* Decide whether we can use the XTest extension */
    if (UseXTest)
        UseXTest = XQueryExtension(disp, "XTEST", &op, &ev, &er);
    if (Debug) {
        if (UseXTest)
            printf("Using XTest Extension\n");
        else
            printf("No XTest Extension: Using XSendEvent\n");
    }
    
    if (!UseStdin) {
        for (i=1; i < argc; ++i) {
            simulateKeyPressForString(disp, argv[i]);
            if (i < argc-1)
                simulateKeyPress(disp, " ");
        }
    }
    else {  /* use standard input ; this is highly inefficient */
        char buffer[BUFSIZE];
        
        do {
            fgets(buffer, BUFSIZE, stdin);
            /* don't print empty strings */
            if (strlen(buffer) > 0) 
                simulateKeyPressForString(disp, buffer);            
            /* clear the string */
            buffer[0] = '\0';
        } while (!feof(stdin));
    }
    return 0;
}


