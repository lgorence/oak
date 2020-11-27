# Oak
Oak is a [Wayland](https://wayland.freedesktop.org/) compositor using the [wlroots](https://github.com/swaywm/wlroots/) library.
The primary target device is for smartphones and tablets. I'm still heavily learning about Wayland and wlroots, so a lot of the code
here is likely not ideal for reference.

## Developing
This project is primarily targeted at the Pinephone. However, wlroots allows for easy development
inside of an existing Wayland or X11 session. If you configure CMake with `-DINCEPTION`, the project will
be forced to a custom mode that emulates that of a Pinephone (while also fitting on a landscape screen).

## Shell
[Acorn](https://github.com/lgorence/acorn) is a shell that I am developing alongside of Oak.
