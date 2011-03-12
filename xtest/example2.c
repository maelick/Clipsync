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
    XSelectionRequestEvent *req;
    XEvent e, respond;
    for(;;) {
        XNextEvent(dpy, &e);
        if (e.type == MapNotify) break;
    }
    XFlush(dpy);
    //
    Atom a1, a2, a3, type;
    XSelectInput(dpy, w, StructureNotifyMask+ExposureMask);
    int format, result;
    unsigned long len, bytes_left, dummy;
    unsigned char *data;
    Window Sown;
    for (int ii = 0; ii < 50; ii++) {
        XSetSelectionOwner (dpy, XA_PRIMARY, w, CurrentTime);
        XFlush (dpy);
        XNextEvent (dpy, &e);
        if (e.type == SelectionRequest)
            //
            // Somebody wants our data
            //
        {
            req=&(e.xselectionrequest);
            printf ("Selection Request from Mr %i I am %i\n",
                    (int)e.xselection.requestor, (int)w);
            printf ("prop:%i tar:%i sel:%i\n", req->property,
                    req->target, req->selection);
            if (req->target == XA_STRING)
            {
                XChangeProperty (dpy,
                                 req->requestor,
                                 req->property,
                                 XA_STRING,
                                 8,
                                 PropModeReplace,
                                 (unsigned char*) "It Works",
                                 8);
                respond.xselection.property=req->property;
            }
            else // Strings only please
            {
                printf ("No String %i\n",
                        (int)req->target);
                respond.xselection.property= None;
            }
            respond.xselection.type= SelectionNotify;
            respond.xselection.display= req->display;
            respond.xselection.requestor= req->requestor;
            respond.xselection.selection=req->selection;
            respond.xselection.target= req->target;
            respond.xselection.time = req->time;
            XSendEvent (dpy, req->requestor,0,0,&respond);
            XFlush (dpy);
        }
    }
}
