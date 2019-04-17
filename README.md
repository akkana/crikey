# crikey
Crikey! A program to generate typed key events on Linux

My husband used to complain about a piece of functionality he missed
from Mac and Windows QuickKeys: the ability to assign a key shortcut
to a string. For example, he might want to be able to store his
username on F1 and then use it quickly in any application. He might
even want to do the same for his password, setting aside the obvious
security issues.

Linux, despite its generous configurability in nearly every other
respect, seems to lack this ability. So I wrote Crikey! Conveniently
Repeated Input Key -- a little program that simulates key events.

```
$ crikey -h
crikey! version 0.8.4
        by Akkana Peck, http://shallowsky.com/software/crikey

Usage: crikey [-itxr] [-sS sleeptime] string...
        -s seconds: sleep time before sending
        -S milliseconds: sleep time before sending
        -i: Interactive (read input from stdin)
        -t: Use XTest to send events (default)
        -x: Use XSendEvent to send events
        -r: Send events to root window (only with XSendEvent)
        -l: Show long (more detailed) help
        -d: Show debug messages
```

```
$ crikey -l
Crikey input options:

Normal letters: just type the letters, e.g. crikey my long string.

For most of the following special cases, it's best to use
single quotes so the shell won't strip out the backslashes.

Control characters: ^A sends a Control-A.
To send a plain ^, use ^^.
Numeric ASCII codes: \12
Special codes: \t tab, \b backspace, \n newline, \r return, \d delete, \e escape
Modifier keys: \S for shift, \C control, \A alt,
  \M or \W for the "Windows" key.
Special symbols: \(Return\) (defined in /usr/include/X11/keysymdef.h)
  but only those defined on your keyboard will likely work.)

Examples:
  crikey '\CL': print a control-L (clear the screen).
  crikey '^L': print a control-L (clear the screen).
  crikey "echo foo \(greater\) /dev/null"
  crikey "wall\nHello, world\n^D"
  crikey '\(Up\)': send an up-arrow.
  crikey -t "\A\t":
    Should change the active window (in most window managers).
    This needs XTest (-t) and doesn't work with XSendEvent.
  crikey '~\Cz':
    When sshed into another computer: should suspend the ssh.
  crikey 'oHere is a line\e\e'
    In vim, not in insert mode. Should add a line, leave
    insert mode then beep if your machine supports that

```

For more details, see the
[Crikey! page on my website](http://shallowsky.com/software/crikey/).
