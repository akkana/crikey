/*
 * Key Event simulation program.
 * Plug in to your window manager's key handler,
 * to allow for binding passwords, etc. to a key.
 *
 * Copyright 2003-2009 by Akkana Peck, http://www.shallowsky.com/software/
 * Other contributors:
 *    Glen Smith, 2008
 *    Efraim Feinstein, 2004
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

#define VERSION "0.8.3"

static int Debug = 0;
static int UseXTest = 1;
static int UseStdin = 0;
static int UseRootWin = 0;

/* size of the buffer for reading from stdin */
#define BUFSIZE 256

/*
 * Non-printable characters we can handle.
 * List of character definitions is in /usr/include/X11/keysymdef.h
 * if you need to add anything.
 */

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
    { '\010', "BackSpace" },  // \b doesn't work
    { '\177', "Delete" },
    { '\27', "Escape" },      // \e doesn't work
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

    /* Filter out keysyms, any string longer than 1 char */
    /* XXX This will probably break if we ever support multibyte chars. */
    if (keyname[1] != '\0')
        return 0;

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

static void simulateKeyPress(Display *disp, KeySym keysym, int modmask)
{
    KeyCode keycode = 0;

    keycode = XKeysymToKeycode(disp, keysym);
    if (keycode == 0) {
        printf("crikey: Can't simulate keysym %ld: no keycode\n", keysym);
        return;
    }

    if (Debug)
        printf("keysym is %ld, keycode is %d, modmask is 0x%x\n",
               keysym, keycode, modmask);

    if (UseXTest) {
        static KeyCode shiftKeycode = 0;
        static KeyCode ctrlKeycode = 0;
        static KeyCode altKeycode = 0;
        static KeyCode metaKeycode = 0;

        if (Debug)
            printf("XTest wth mask = 0x%x\n", modmask);
        XTestGrabControl(disp, True);

#define FAKE_KEY(dpy,k,p,dl) { \
            if (Debug) \
                printf("Calling XTestFakeKeyEvent(%p, %d, %d, %d)\n", \
                       dpy, k, p, dl); \
            XTestFakeKeyEvent(dpy, k, p, dl); \
    }

        /* Handle modifier key presses */
        if (modmask & ShiftMask) {
            if (shiftKeycode == 0) {
                shiftKeycode = XKeysymToKeycode(disp, XK_Shift_L);
                if (Debug)
                    printf("Keycode for shift is %d\n", shiftKeycode);
            }
            FAKE_KEY(disp, shiftKeycode, True, 0);   /* shift press */
        }
        if (modmask & ControlMask) {
            if (ctrlKeycode == 0) {
                ctrlKeycode = XKeysymToKeycode(disp, XK_Control_L);
                if (Debug)
                    printf("Keycode for ctrl is %d\n", ctrlKeycode);
            }
            FAKE_KEY(disp, ctrlKeycode, True, 0);    /* ctrl press */
        }

        if (modmask & Mod1Mask) {
            if (altKeycode == 0) {
                altKeycode = XKeysymToKeycode(disp, XK_Alt_L);
                if (Debug)
                    printf("Keycode for alt is %d\n", altKeycode);
            }
            FAKE_KEY(disp, altKeycode, True, 0);     /* alt press */
        }

        if (modmask & Mod4Mask) {
            if (metaKeycode == 0) {
                metaKeycode = XKeysymToKeycode(disp, XK_Super_L);
                if (Debug)
                    printf("Keycode for meta/windows is %d\n", metaKeycode);
            }
            FAKE_KEY(disp, metaKeycode, True, 0);    /* win press */
        }

        FAKE_KEY(disp, keycode, True, 0);            /* key press */
        FAKE_KEY(disp, keycode, False, 0);           /* key release */

        /* Handle modifier key releases */
        if (modmask & Mod4Mask)
            FAKE_KEY(disp, metaKeycode, False, 0);   /* win rel */
        if (modmask & Mod1Mask)
            FAKE_KEY(disp, altKeycode, False, 0);    /* alt rel */
        if (modmask & ControlMask)
            FAKE_KEY(disp, ctrlKeycode, False, 0);   /* ctrl rel */
        if (modmask & ShiftMask)
            FAKE_KEY(disp, shiftKeycode, False, 0);  /* shift rel */

        XSync(disp, False);
        XTestGrabControl(disp, False);
    }
    else {
        /* Use XSendEvent instead of XTest */
        XKeyEvent kevent;
        Window focuswin;

        /* Crikey used to get the focused window and send events
         * directly there. But then you can't send commands to your
         * window manager, like alt-tab. Sending to the root window
         * gives the window manager a chance to intercept first.
         */
        if (UseRootWin) {
            if (Debug) printf("Sending to root window\n");
            focuswin = DefaultRootWindow(disp);
        }
        else {
            int revert_to;
            XGetInputFocus(disp, &focuswin, &revert_to);
            if (focuswin == 0) {
                printf("No focused window!\n");
                return;
            }
        }

        kevent.display = disp;
        kevent.root = DefaultRootWindow(disp);
        kevent.window = focuswin;
        kevent.subwindow = None;
        kevent.time = CurrentTime;
        kevent.x = 1;
        kevent.y = 1;
        kevent.x_root = 1;
        kevent.y_root = 1;
        kevent.same_screen = TRUE;
        kevent.type = KeyPress;
        kevent.keycode = keycode;
        kevent.state = modmask;
        if (Debug)
            printf("Sending an event with keycode = %d, modifier mask 0x%x\n",
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

#define MAXSYMSIZE 32

static void simulateKeyPressForString(Display* disp, char* s)
{
    KeySym keysym;
    char buf[2];
    char sym[MAXSYMSIZE];
    int i, n;
    int modmask = 0;
    int cont;

    while (*s)
    {
        cont = 0;
        keysym = 0;
        if (*s == '\\' && *(s+1) != '\0') {
            switch (*(++s))
            {
              case '\\':
                  buf[0] = '\\';
                  break;
              case 'S':
                  modmask |= ShiftMask;
                  cont = 1;
                  break;
              case 'C':
                  modmask |= ControlMask;
                  cont = 1;
                  break;
              case 'A':
                  modmask |= Mod1Mask;
                  cont = 1;
                  break;
              case 'M':
              case 'W':
                  modmask |= Mod4Mask;
                  cont = 1;
                  break;
              case 'n':
                  buf[0] = '\n';
                  break;
              case 'r':
                  buf[0] = '\r';
                  break;
              case 't':
                  buf[0] = '\t';
                  break;
              case 'b':
                  buf[0] = '\010';
                  break;
              case 'd':
                  buf[0] = '\177';
                  break;
              case 'e':
                  buf[0] = '\27';
                  break;
              case '0':  case '1':  case '2':  case '3':  case '4':
              case '5':  case '6':  case '7':  case '8':  case '9':
                  for (i = n = 0; isdigit(s[0]); ++s)
                      n = n * 10 + s[0] - '0';
                  buf[0] = n;
                  if (Debug) printf("Numeric character %d\n", n);
                  --s;
                  break;
              case '(':
                  /* parse a symbolic name */
                  for (i=0, ++s;
                       i < MAXSYMSIZE-1 && s[0] != 0
                       && !(s[0] == '\\' && s[1] == ')');
                       ++i, ++s) {
                      sym[i] = *s;
                  }

                  /* keysym is in sym; parse it now */
                  sym[i] = '\0';
                  if (Debug) {
                      printf("Found symbol %s ...", sym);
                      printf("which has keysym %lu ", XStringToKeysym(sym));
                      printf("and keycode %d\n",
                             (unsigned)XKeysymToKeycode(disp,
                                                        XStringToKeysym(sym)));
                  }
                  buf[0] = 0;
                  keysym = XStringToKeysym(sym);
                  ++s;
                  break;
              default:
                  --s;
                  buf[0] = '\\';
                  break;
            }
        }
        else if (s[0] == '^' && s[1] != '\0') {
            /* Control character */
            if (Debug) printf("Control character\n");
            if (!isalpha(s[1])) {
                buf[0] = '^';
            }
            else if (isupper(s[1]))
                buf[0] = s[1] - 'A' + 1;
            else
                buf[0] = s[1] - 'a' + 1;
            ++s;
            if (Debug)
                printf("Control character ^%c = %ld\n", s[0], keysym);
        }
        else
            buf[0] = *s;

        /* If cont is set, then we've only gotten modifiers
         * and need to loop around again to get the actual character.
         */
        if (cont) {
            ++s;
            continue;
        }

        if (!keysym && buf[0] != 0) {
            buf[1] = '\0';

            /* Evil special cases */

            /* Ctrl-D has to be sent as a control and a d,
             * not an EOF character.
             */
            if (buf[0] == 4) {   /* ctrl-d */
                modmask |= ControlMask;
                buf[0] = 'd';
            }

            /* For some reason, '<' needs special treatment.
             * XStringToKeysym("less") returns 60, and
             * XKeysymToKeycode(60) returns keycode 94
             * but generating an event with keycode 94 will send a '>'
             * The keysym needs to be 44 to get a keycode of 59 to send a '<'.
            if (keysym == 60) {
            */
            if (buf[0] == '<') {
                keysym = 44;
                //keycode = 59;
                //modmask |= ShiftMask;       /* may not need this */
                if (Debug) printf("Special case for >\n");
            }
            else
                keysym = XStringToKeysym(buf);

            if (keysym == 0) {
                int i;
                if (Debug) printf("keysym for char 0x%x was 0\n", buf[0]);
                for (i=0;
                     i < ((sizeof NonPrintables) / (sizeof *NonPrintables));
                     ++i) {
                    if (*buf == NonPrintables[i].ch) {
                        keysym = XStringToKeysym(NonPrintables[i].keySymName);
                        if (Debug)
                            printf("Nonprintable: name = %s, keysym = %ld\n",
                                   NonPrintables[i].keySymName, keysym);
                        break;
                    }
                }
            }
        }

        if (isshift(buf))
            modmask |= ShiftMask;

        if (keysym)
            simulateKeyPress(disp, keysym, modmask);
        else if (Debug) {
            printf("crikey: Can't simulate key '%s'\n", buf);
        }

        ++s;
        modmask = 0;
    }
}

void Usage(void)
{
    printf("crikey! version %s\n", VERSION);
    printf("\tby Akkana Peck, http://shallowsky.com/software/crikey\n\n");
    printf("Usage: crikey [-itxr] [-sS sleeptime] string...\n");
    printf("\t-s seconds: sleep time before sending\n");
    printf("\t-S milliseconds: sleep time before sending\n");
    printf("\t-i: Interactive (read input from stdin)\n");
    printf("\t-t: Use XTest to send events (default)\n");
    printf("\t-x: Use XSendEvent to send events\n");
    printf("\t-r: Send events to root window (only with XSendEvent)\n");
    printf("\t-l: Show long (more detailed) help\n");
    printf("\t-d: Show debug messages\n");
    exit(0);
}

void LongHelp(void)
{
    printf("Crikey input options:\n\n");
    printf("Normal letters: just type the letters, e.g. crikey my long string.\n\n");
    printf("For most of the following special cases, it's best to use\n");
    printf("quotes so the shell won't strip out the backslashes.\n\n");
    printf("Control characters: ^A sends a Control-A\n");
    printf("Numeric ASCII codes: \\12\n");
    printf("Special codes: \\t tab, \\b backspace, \\n newline, \\r return, \\d delete, \\e escape\n");
    printf("Modifier keys: \\S for shift, \\C control, \\A alt,\n");
    printf("  \\M or \\W for the \"Windows\" key.\n");
    printf("Special symbols: \\(Return\\) (defined in /usr/include/X11/keysymdef.h)\n");
    printf("  but only those defined on your keyboard will likely work.)\n");
    printf("\n");
    exit(0);
}

int main(int argc, char** argv)
{
    int i;
    Display* disp = XOpenDisplay(0);
    int op, ev, er;
    int sleeptime = 0;
    int use_usleep = FALSE;

    /* -- means "ignore all flags after this one"
     * so crikey can handle strings starting with a dash.
     */
    while (argc > 1 && argv[1][0] == '-') {
        if (argv[1][1] == '-') {
            --argc;
            ++argv;
            break;
        }

        switch(argv[1][1]) {
          case 'd': // debug mode
              Debug=1;
              break;      
          case 'S':  // millisecond sleep
              use_usleep = TRUE;
          case 's':  // sleep
              if (isdigit(argv[1][2])) {
                  sleeptime = atoi(argv[1]+2);
              }
              else if (argc > 2 && isdigit(argv[2][0])) {
                  sleeptime = atoi(argv[2]);
                  --argc;
                  ++argv;
              }
              else {
                  printf("Sleep how long?\n");
                  Usage();
              }
              if (Debug)
                  printf("Sleeping for %d %sseconds\n", sleeptime,
                         use_usleep ? "milli" : "");
              if (use_usleep)
                  usleep(sleeptime * 1000);
              else
                  sleep(sleeptime);
              break;

          case 'x':  // use XSendEvent, not XTest
              UseXTest = 0;
              break;
          case 't':  // use X Test extension if available.  Default: don't.
              UseXTest = 1;
              break;

          case 'r':  // Send events to the root window, not the focused one.
                     // (only matters for XSendEvent)
              UseRootWin = 1;
              break;
          case 'i':  // take string from standard input
              UseStdin = 1;
              break;
          case 'l':
              LongHelp();
          default:
              Usage();
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
            printf("Using XSendEvent\n");
    }
    
    if (!UseStdin) {
        KeySym space_keysym = XStringToKeysym(" ");
        for (i=1; i < argc; ++i) {
            simulateKeyPressForString(disp, argv[i]);
            if (i < argc-1)
                simulateKeyPress(disp, space_keysym, 0);
        }
    }
    else {  /* use standard input ; this is highly inefficient */
        char buffer[BUFSIZE];
        
        do {
            if (fgets(buffer, BUFSIZE, stdin) != 0) {
                /* don't print empty strings */
                if (strlen(buffer) > 0) 
                    simulateKeyPressForString(disp, buffer);
                /* clear the string */
                buffer[0] = '\0';
            }
        } while (!feof(stdin));
    }
    return 0;
}


