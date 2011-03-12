from Xlib import display, Xatom, X
import time

def paste():
    d = display.Display()
    owner = d.get_selection_owner(Xatom.PRIMARY) # Si = 0, erreur
    print owner
    owner.convert_selection(Xatom.PRIMARY, Xatom.STRING, 0, int(time.time()))
    d.flush()
    p1 = owner.get_property(Xatom.STRING, X.AnyPropertyType, 0, 0)
    p2 = owner.get_property(Xatom.STRING, X.AnyPropertyType, 0, p1.bytes_after)
    return p2.value

def copy(text):
    d = display.Display()
    w = display.xobject.drawable.Window()
