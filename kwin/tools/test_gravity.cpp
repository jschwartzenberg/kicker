// tests for window gravity

#define INITIAL_POSITION_TEST

#include <iostream>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

const int gravities[ 10 ] =
    {
    NorthWestGravity, 
    NorthGravity,  
    NorthEastGravity,
    WestGravity,      
    CenterGravity, 
    EastGravity,
    SouthWestGravity,
    SouthGravity,
    SouthEastGravity,
    StaticGravity
    };
    
const char* const gravity_names[ 10 ] =
    {
    "NW", "N", "NE", "W", "C", "E", "SW", "S", "SE", "ST"
    };
    
Display* dpy = NULL;

int get_gravity( const char* name )
    {
    for( int i = 0;
         i < 10;
         ++i )
        if( strcmp( name, gravity_names[ i ] ) == 0 )
            return gravities[ i ];
    cerr << "Wrong gravity name" << endl;
    exit( 1 );
    }

void initial_position_test( const char* gravity )
    {
    XSetWindowAttributes attrs;
    XSizeHints hints;
    hints.flags = USPosition | PWinGravity;
    hints.win_gravity = get_gravity( gravity );
    Window w = XCreateWindow( dpy, DefaultRootWindow( dpy ), 100, 100, 200, 100, 0, CopyFromParent, CopyFromParent,
        CopyFromParent, 0, &attrs );
    XSetWMNormalHints( dpy, w, &hints );
    XSelectInput( dpy, w, StructureNotifyMask );
    XMapWindow( dpy, w );
    for(;;)
        {
        XEvent ev;
        XNextEvent( dpy, &ev );
        if( ev.type == ConfigureNotify )
            {
            cout << "CONFIGURENOTIFY:" << ev.xany.send_event << ":" << ev.xconfigure.x << ":" << ev.xconfigure.y
                << ":" << ev.xconfigure.width << ":" << ev.xconfigure.height << endl;
            Window root, child;
            int x, x_local, y, y_local;
            unsigned int width, height, border, depth;
            XGetGeometry( dpy, w, &root, &x_local, &y_local, &width, &height, &border, &depth );
            XTranslateCoordinates( dpy, w, root, 0, 0, &x, &y, &child );
            cout << "GEOMETRY:" << x << ":" << y << ":" << width << ":" << height << ":(" << x_local << ":" << y_local << ")" << endl;
            }
        }
    }

int main( int argc, char* argv[] )
    {
    dpy = XOpenDisplay( NULL );
#ifdef INITIAL_POSITION_TEST
    if( argc != 2 )
        {
        cerr << "specify gravity" << endl;
        exit( 1 );
        }
    initial_position_test( argv[ 1 ] );
#endif
    XCloseDisplay( dpy );
    }
