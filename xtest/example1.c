#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

main()
{
    Display *dpy = XOpenDisplay(NULL);
    assert(dpy);
    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                                   200, 100, 0, 0, 0);
    XSelectInput(dpy, w, StructureNotifyMask);
    XMapWindow(dpy, w);
    XEvent e;
    for(;;) {
        XNextEvent(dpy, &e);
        if (e.type == MapNotify) break;
    }
    XFlush(dpy);
    // Copy from application
    Atom a1, a2, type;
    XSelectInput(dpy, w, StructureNotifyMask+ExposureMask);
    int format, result;
    unsigned long len, bytes_left, dummy;
    unsigned char *data;
    Window Sown;
    for (int ii = 0; ii < 50; ii++) {
        Sown = XGetSelectionOwner (dpy, XA_PRIMARY);
        printf ("Selection owner%i\n", (int)Sown);
        if (Sown != None) {
            XConvertSelection (dpy, XA_PRIMARY, XA_STRING, None,
                               Sown, CurrentTime);
            XFlush (dpy);
            //
            // Do not get any data, see how much data is there
            //
            XGetWindowProperty (dpy, Sown,
                                XA_STRING,   // Tricky..
                                0, 0,    // offset - len
                                0,    // Delete 0==FALSE
                                AnyPropertyType,  //flag
                                &type,  // return type
                                &format,  // return format
                                &len, &bytes_left,  //that
                                &data);
            printf ("type:%i len:%i format:%i byte_left:%i\n",
                    (int)type, len, format, bytes_left);
            // DATA is There
            if (bytes_left > 0)
            {
                result = XGetWindowProperty (dpy, Sown,
                                             XA_STRING, 0,bytes_left,0,
                                             AnyPropertyType, &type,&format,
                                             &len, &dummy, &data);
                if (result == Success)
                    printf ("DATA IS HERE!!```%s'''\n",
                            data);
                else printf ("FAIL\n");
                XFree (data);
            }
        }
        sleep(2);
    }
}
