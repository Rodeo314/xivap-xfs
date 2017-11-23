#ifndef _XPLMDisplay_h_
#define _XPLMDisplay_h_

/*
 * Copyright 2005-2012 Sandy Barbour and Ben Supnik
 * 
 * All rights reserved.  See license.txt for usage.
 * 
 * X-Plane SDK Version: 2.1.1                                                  
 *
 */

/*
 * This API provides the basic hooks to draw in X-Plane and create user 
 * interface. All X-Plane drawing is done in OpenGL.  The X-Plane plug-in 
 * manager takes care of properly setting up the OpenGL context and matrices.  
 * You do not decide when in your code's  execution to draw; X-Plane tells you 
 * when it is ready to have your plugin draw. 
 * 
 * X-Plane's drawing strategy is straightforward: every "frame" the screen is 
 * rendered by drawing the 3-d scene (dome, ground, objects, airplanes, etc.)  
 * and then drawing the cockpit on top of it.  Alpha blending is used to 
 * overlay the cockpit over the world (and the gauges over the panel, etc.). 
 * X-Plane user interface elements (including windows like the map, the main 
 * menu, etc.) are then drawn on top of the cockpit. 
 * 
 * There are two ways you can draw: directly and in a window. 
 * 
 * Direct drawing involves drawing to the screen before or after X-Plane 
 * finishes a phase of drawing.  When you draw directly, you can specify 
 * whether X-Plane is to complete this phase or not.  This allows you to do 
 * three things: draw before X-Plane does (under it), draw after X-Plane does 
 * (over it), or draw instead of X-Plane. 
 * 
 * To draw directly, you register a callback and specify which phase you want 
 * to intercept.  The plug-in manager will call you over and over to draw that 
 * phase. 
 * 
 * Direct drawing allows you to override scenery, panels, or anything.  Note 
 * that  you cannot assume that you are the only plug-in drawing at this 
 * phase. 
 * 
 * Window drawing provides slightly higher level functionality.  With window 
 * drawing you create a window that takes up a portion of the screen.  Window 
 * drawing is always two dimensional.  Window drawing is front-to-back 
 * controlled; you can  specify that you want your window to be brought on 
 * top, and other plug-ins may put their window on top of you.  Window drawing 
 * also allows you to sign up for key presses and receive mouse clicks. 
 * 
 * There are three ways to get keystrokes: 
 * 
 * If you create a window, the window can take keyboard focus.  It will then 
 * receive all keystrokes.  If no window has focus, X-Plane receives 
 * keystrokes.  Use this to implement typing in dialog boxes, etc.  Only one 
 * window may have focus at a time; your window will be notified if it loses 
 * focus. 
 * 
 * If you need to associate key strokes with commands/functions in your 
 * plug-in, use a hot key.  A hoy is a key-specific callback.  Hotkeys are 
 * sent based on virtual key strokes, so any key may be distinctly mapped with 
 * any modifiers.  Hot keys  can be remapped by other plug-ins.  As a plug-in, 
 * you don't have to worry about  what your hot key ends up mapped to; other 
 * plug-ins may provide a UI for remapping keystrokes.  So hotkeys allow a 
 * user to resolve conflicts and customize keystrokes. 
 * 
 * If you need low level access to the keystroke stream, install a key 
 * sniffer.  Key sniffers can be installed above everything or right in front 
 * of the sim.                                                                 
 *
 */

#include "XPLMDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * DRAWING CALLBACKS
 ***************************************************************************/
/*
 * Basic drawing callbacks, for low level intercepting of render loop. The 
 * purpose of drawing callbacks is to provide targeted additions or  
 * replacements to X-Plane's graphics environment (for example, to add extra 
 * custom objects, or replace drawing of the AI aircraft).  Do not assume that 
 * the drawing callbacks will be called in the order implied by the 
 * enumerations. Also do not assume that each drawing phase ends before 
 * another begins; they may be nested.                                         
 *
 */


/*
 * XPLMDrawingPhase
 * 
 * This constant indicates which part of drawing we are in.  Drawing is done 
 * from the back to the front.  We get a callback before or after each item. 
 * Metaphases provide access to the beginning and end of the 3d (scene) and 2d 
 * (cockpit) drawing in a manner that is independent of new phases added  via 
 * X-Plane implementation. 
 * 
 * WARNING: As X-Plane's scenery evolves, some drawing phases may cease to  
 * exist and new ones may be invented.  If you need a particularly specific 
 * use of these codes, consult Austin and/or be prepared to revise your code 
 * as X-Plane evolves.                                                         
 *
 */
enum {
     /* This is the earliest point at which you can draw in 3-d.                    */
     xplm_Phase_FirstScene                    = 0

     /* Drawing of land and water.                                                  */
    ,xplm_Phase_Terrain                       = 5

     /* Drawing runways and other airport detail.                                   */
    ,xplm_Phase_Airports                      = 10

     /* Drawing roads, trails, trains, etc.                                         */
    ,xplm_Phase_Vectors                       = 15

     /* 3-d objects (houses, smokestacks, etc.                                      */
    ,xplm_Phase_Objects                       = 20

     /* External views of airplanes, both yours and the AI aircraft.                */
    ,xplm_Phase_Airplanes                     = 25

     /* This is the last point at which you can draw in 3-d.                        */
    ,xplm_Phase_LastScene                     = 30

     /* This is the first phase where you can draw in 2-d.                          */
    ,xplm_Phase_FirstCockpit                  = 35

     /* The non-moving parts of the aircraft panel.                                 */
    ,xplm_Phase_Panel                         = 40

     /* The moving parts of the aircraft panel.                                     */
    ,xplm_Phase_Gauges                        = 45

     /* Floating windows from plugins.                                              */
    ,xplm_Phase_Window                        = 50

     /* The last change to draw in 2d.                                              */
    ,xplm_Phase_LastCockpit                   = 55

#if defined(XPLM200)
     /* Deprecated as of XPLM300; Use the full-blown map API instead.               */
    ,xplm_Phase_LocalMap3D                    = 100

#endif /* XPLM200 */
#if defined(XPLM200)
     /* Deprecated as of XPLM300; Use the full-blown map API instead.               */
    ,xplm_Phase_LocalMap2D                    = 101

#endif /* XPLM200 */
#if defined(XPLM200)
     /* Deprecated as of XPLM300; Use the full-blown map API instead.               */
    ,xplm_Phase_LocalMapProfile               = 102

#endif /* XPLM200 */

};
typedef int XPLMDrawingPhase;

/*
 * XPLMDrawCallback_f
 * 
 * This is the prototype for a low level drawing callback.  You are passed in 
 * the phase and whether it is before or after.  If you are before the phase, 
 * return 1 to let X-Plane draw or 0 to suppress X-Plane drawing.  If you are 
 * after the phase the return value is ignored. 
 * 
 * Refcon is a unique value that you specify when registering the callback, 
 * allowing you to slip a pointer to your own data to the callback. 
 * 
 * Upon entry the OpenGL context will be correctly set up for you and OpenGL 
 * will be in 'local' coordinates for 3d drawing and panel coordinates for 2d 
 * drawing.  The OpenGL state (texturing, etc.) will be unknown.               
 *
 */
typedef int (* XPLMDrawCallback_f)(
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inIsBefore,    
                                   void *               inRefcon);    

/*
 * XPLMKeySniffer_f
 * 
 * This is the prototype for a low level key-sniffing function.  Window-based 
 * UI _should not use this_!  The windowing system provides high-level 
 * mediated keyboard access.  By comparison, the key sniffer provides low 
 * level keyboard access. 
 * 
 * Key sniffers are provided to allow libraries to provide non-windowed user  
 * interaction.  For example, the MUI library uses a key sniffer to do pop-up 
 * text entry. 
 * 
 * inKey is the character pressed, inRefCon is a value you supply during 
 * registration. Return 1 to pass the key on to the next sniffer, the window 
 * mgr, X-Plane, or whomever is down stream.  Return 0 to consume the key. 
 * 
 * Warning: this API declares virtual keys as a signed character; however the 
 * VKEY #define macros in XPLMDefs.h define the vkeys using unsigned values 
 * (that is 0x80 instead of -0x80).  So you may need to cast the incoming vkey 
 * to an unsigned char to get correct comparisons in C.                        
 *
 */
typedef int (* XPLMKeySniffer_f)(
                                   char                 inChar,    
                                   XPLMKeyFlags         inFlags,    
                                   char                 inVirtualKey,    
                                   void *               inRefcon);    

/*
 * XPLMRegisterDrawCallback
 * 
 * This routine registers a low level drawing callback.  Pass in the phase you 
 * want to be called for and whether you want to be called before or after.  
 * This routine returns 1 if the registration was successful, or 0 if the 
 * phase does not exist in this version of X-Plane.  You may register a 
 * callback multiple times for the same or different phases as long as the 
 * refcon is unique each time.                                                 
 *
 */
XPLM_API int                  XPLMRegisterDrawCallback(
                                   XPLMDrawCallback_f   inCallback,    
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inWantsBefore,    
                                   void *               inRefcon);    

/*
 * XPLMUnregisterDrawCallback
 * 
 * This routine unregisters a draw callback.  You must unregister a callback 
 * for each  time you register a callback if you have registered it multiple 
 * times with different refcons.  The routine returns 1 if it can find the 
 * callback to unregister, 0 otherwise.                                        
 *
 */
XPLM_API int                  XPLMUnregisterDrawCallback(
                                   XPLMDrawCallback_f   inCallback,    
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inWantsBefore,    
                                   void *               inRefcon);    

/*
 * XPLMRegisterKeySniffer
 * 
 * This routine registers a key sniffing callback.  You specify whether you 
 * want to sniff before the window system, or only sniff keys the window 
 * system does not consume.  You should ALMOST ALWAYS sniff non-control keys 
 * after the window system.  When the window system consumes a key, it is 
 * because the user has "focused" a window.  Consuming the key or taking 
 * action based on the key will produce very weird results.  Returns 1 if 
 * successful.                                                                 
 *
 */
XPLM_API int                  XPLMRegisterKeySniffer(
                                   XPLMKeySniffer_f     inCallback,    
                                   int                  inBeforeWindows,    
                                   void *               inRefcon);    

/*
 * XPLMUnregisterKeySniffer
 * 
 * This routine unregisters a key sniffer.  You must unregister a key sniffer 
 * for every time you register one with the exact same signature.  Returns 1 
 * if successful.                                                              
 *
 */
XPLM_API int                  XPLMUnregisterKeySniffer(
                                   XPLMKeySniffer_f     inCallback,    
                                   int                  inBeforeWindows,    
                                   void *               inRefcon);    

/***************************************************************************
 * WINDOW API
 ***************************************************************************/
/*
 * The window API provides a high-level abstraction for drawing with UI 
 * interaction. 
 * 
 * Windows may operate in one of two modes: legacy (for plugins compiled 
 * against old versions of the XPLM, as well as windows created via the 
 * deprecated XPLMCreateWindow() function, rather than XPLMCreateWindowEx()), 
 * or modern (for windows compiled against the XPLM300 or newer API, and 
 * created via XPLMCreateWindowEX()). 
 * 
 * Modern windows have access to new X-Plane 11 windowing features, like 
 * support for new positioning modes (including being "popped out" into their 
 * own first-class window in the operating system). They can also optionally 
 * be decorated in the style of X-Plane 11 windows (like the map). 
 * 
 * Modern windows operate in "boxel" units. A boxel ("box of pixels") is a 
 * unit of virtual pixels which, depending on X-Plane's scaling, may 
 * correspond to an arbitrary NxN "box" of real pixels on screen. Because 
 * X-Plane handles this scaling automatically, you can effectively treat the 
 * units as though you were simply drawing in pixels, and know that when 
 * X-Plane is running with 150% or 200% scaling, your drawing will be 
 * automatically scaled (and likewise all mouse coordinates, screen bounds, 
 * etc. will also be auto-scaled). 
 * 
 * In contrast, legacy windows draw in true screen pixels, and thus tend to 
 * look quite small when X-Plane is operating in a scaled mode. 
 * 
 * Legacy windows have their origin in the lower left of the main X-Plane 
 * window, while modern windows have their origin in the lower left of the 
 * entire global desktop space (since they are not constrained to the main 
 * window). In both cases, x increases as you move left, and y increases as 
 * you move up.                                                                
 *
 */


/*
 * XPLMMouseStatus
 * 
 * When the mouse is clicked, your mouse click routine is called repeatedly.  
 * It is first called with the mouse down message.  It is then called zero or 
 * more times with the mouse-drag message, and finally it is called once with 
 * the mouse up message.  All of these messages will be directed to the same 
 * window.                                                                     
 *
 */
enum {
     xplm_MouseDown                           = 1

    ,xplm_MouseDrag                           = 2

    ,xplm_MouseUp                             = 3


};
typedef int XPLMMouseStatus;

#if defined(XPLM200)
/*
 * XPLMCursorStatus
 * 
 * XPLMCursorStatus describes how you would like X-Plane to manage the cursor. 
 * See XPLMHandleCursor_f for more info.                                       
 *
 */
enum {
     /* X-Plane manages the cursor normally, plugin does not affect the cusrsor.    */
     xplm_CursorDefault                       = 0

     /* X-Plane hides the cursor.                                                   */
    ,xplm_CursorHidden                        = 1

     /* X-Plane shows the cursor as the default arrow.                              */
    ,xplm_CursorArrow                         = 2

     /* X-Plane shows the cursor but lets you select an OS cursor.                  */
    ,xplm_CursorCustom                        = 3


};
typedef int XPLMCursorStatus;
#endif /* XPLM200 */

/*
 * XPLMWindowID
 * 
 * This is an opaque identifier for a window.  You use it to control your 
 * window. When you create a window, you will specify callbacks to handle 
 * drawing and mouse interaction, etc.                                         
 *
 */
typedef void * XPLMWindowID;

/*
 * XPLMDrawWindow_f
 * 
 * This function handles drawing.  You are passed in your window and its 
 * refcon. Draw the window.  You can use XPLM functions to find the current 
 * dimensions of your window, etc.  When this callback is called, the OpenGL 
 * context will be set properly for cockpit drawing. NOTE: Because you are 
 * drawing your window over a background, you can make a translucent window 
 * easily by simply not filling in your entire window's bounds.                
 *
 */
typedef void (* XPLMDrawWindow_f)(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon);    

#if defined(XPLM300)
/*
 * XPLMReceiveMonitorBoundsGlobal_f
 * 
 * This function is informed of the global bounds (in boxels) of a particular 
 * monitor within the X-Plane global desktop space. Note that X-Plane must be 
 * running in full screen on a monitor in order for it to be passed to you in 
 * this callback.                                                              
 *
 */
typedef void (* XPLMReceiveMonitorBoundsGlobal_f)(
                                   int                  inMonitorIndex,    
                                   int                  inLeftBx,    
                                   int                  inTopBx,    
                                   int                  inRightBx,    
                                   int                  inBottomBx,    
                                   void *               refcon);    
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMReceiveMonitorBoundsOS_f
 * 
 * This function is informed of the global bounds (in pixels) of a particular 
 * monitor within the operating system's global desktop space. Note that a 
 * monitor index being passed to you here does not indicate that X-Plane is 
 * running in full screen on this monitor.                                     
 *
 */
typedef void (* XPLMReceiveMonitorBoundsOS_f)(
                                   int                  inMonitorIndex,    
                                   int                  inLeftPx,    
                                   int                  inTopPx,    
                                   int                  inRightPx,    
                                   int                  inBottomPx,    
                                   void *               refcon);    
#endif /* XPLM300 */

/*
 * XPLMHandleKey_f
 * 
 * This function is called when a key is pressed or keyboard focus is taken 
 * away from your window.  If losingFocus is 1, you are losing the keyboard 
 * focus, otherwise a key was pressed and inKey contains its character.  You 
 * are also passewd your window and a  refcon. Warning: this API declares 
 * virtual keys as a signed character; however the VKEY #define macros in 
 * XPLMDefs.h define the vkeys using unsigned values (that is 0x80 instead of 
 * -0x80).  So you may need to cast the incoming vkey to an unsigned char to 
 * get correct comparisons in C.                                               
 *
 */
typedef void (* XPLMHandleKey_f)(
                                   XPLMWindowID         inWindowID,    
                                   char                 inKey,    
                                   XPLMKeyFlags         inFlags,    
                                   char                 inVirtualKey,    
                                   void *               inRefcon,    
                                   int                  losingFocus);    

/*
 * XPLMHandleMouseClick_f
 * 
 * You receive this call when the mouse button is pressed down or released.  
 * Between then these two calls is a drag.  You receive the x and y of the 
 * click, your window,  and a refcon.  Return 1 to consume the click, or 0 to 
 * pass it through. 
 * 
 * WARNING: passing clicks through windows (as of this writing) causes mouse 
 * tracking problems in X-Plane; do not use this feature! 
 * 
 * The units for x and y values match the units used in your window. Thus, for 
 * "modern" windows (those created via XPLMCreateWindowEx() and compiled 
 * against the XPLM300 library), the units are boxels, while legacy windows 
 * will get pixels. Legacy windows have their origin in the lower left of the 
 * main X-Plane window, while modern windows have their origin in the lower 
 * left of the global desktop space. In both cases, x increases as you move 
 * left, and y increases as you move up.                                       
 *
 */
typedef int (* XPLMHandleMouseClick_f)(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   XPLMMouseStatus      inMouse,    
                                   void *               inRefcon);    

#if defined(XPLM200)
/*
 * XPLMHandleCursor_f
 * 
 * The SDK calls your cursor status callback when the mouse is over your 
 * plugin window.  Return a cursor status code to indicate how you would like 
 * X-Plane to manage the cursor.  If you return xplm_CursorDefault, the SDK 
 * will try lower-Z-order plugin windows, then let the sim manage the cursor. 
 * 
 * Note: you should never show or hide the cursor yourself - these APIs are 
 * typically reference-counted and thus  cannot safely and predictably be used 
 * by the SDK.  Instead return one of xplm_CursorHidden to hide the cursor or 
 * xplm_CursorArrow/xplm_CursorCustom to show the cursor. 
 * 
 * If you want to implement a custom cursor by drawing a cursor in OpenGL, use 
 * xplm_CursorHidden to hide the OS cursor and draw the cursor using a 2-d 
 * drawing callback (after xplm_Phase_Window is probably a good choice).  If 
 * you want to use a custom OS-based cursor, use xplm_CursorCustom to ask 
 * X-Plane to show the cursor but not affect its image.  You can then use an 
 * OS specific call like SetThemeCursor (Mac) or SetCursor/LoadCursor 
 * (Windows). 
 * 
 * The units for x and y values match the units used in your window. Thus, for 
 * "modern" windows (those created via XPLMCreateWindowEx() and compiled 
 * against the XPLM300 library), the units are boxels, while legacy windows 
 * will get pixels. Legacy windows have their origin in the lower left of the 
 * main X-Plane window, while modern windows have their origin in the lower 
 * left of the global desktop space. In both cases, x increases as you move 
 * left, and y increases as you move up.                                       
 *
 */
typedef XPLMCursorStatus (* XPLMHandleCursor_f)(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   void *               inRefcon);    
#endif /* XPLM200 */

#if defined(XPLM200)
/*
 * XPLMHandleMouseWheel_f
 * 
 * The SDK calls your mouse wheel callback when one of the mouse wheels is 
 * turned within your window.  Return 1 to consume the  mouse wheel clicks or 
 * 0 to pass them on to a lower window.  (You should consume mouse wheel 
 * clicks even if they do nothing if your window appears opaque to the user.)  
 * The number of clicks indicates how far the wheel was turned since the last 
 * callback. The wheel is 0 for the vertical axis or 1 for the horizontal axis 
 * (for OS/mouse combinations that support this). 
 * 
 * The units for x and y values match the units used in your window. Thus, for 
 * "modern" windows (those created via XPLMCreateWindowEx() and compiled 
 * against the XPLM300 library), the units are boxels, while legacy windows 
 * will get pixels. Legacy windows have their origin in the lower left of the 
 * main X-Plane window, while modern windows have their origin in the lower 
 * left of the global desktop space. In both cases, x increases as you move 
 * left, and y increases as you move up.                                       
 *
 */
typedef int (* XPLMHandleMouseWheel_f)(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   int                  wheel,    
                                   int                  clicks,    
                                   void *               inRefcon);    
#endif /* XPLM200 */

#if defined(XPLM300)
/*
 * XPLMWindowLayer
 * 
 * XPLMWindowLayer describes where in the ordering of windows X-Plane should 
 * place a particular window. Windows in higher layers cover windows in lower 
 * layers. So, a given window might be at the top of its particular layer, but 
 * it might still be obscured by a window in a higher layer. (This happens 
 * frequently when floating windows, like X-Plane's map, are covered by a 
 * modal alert.) 
 * 
 * Your window's layer can only be specified when you create the window (in 
 * the XPLMCreateWindow_t you pass to XPLMCreateWindowEx()). For this reason, 
 * layering only applies to windows created with new X-Plane 11 GUI features. 
 * (Windows created using the older XPLMCreateWindow(), or windows compiled 
 * against a pre-XPLM300 version of the SDK will simply be placed in the 
 * floating window layer.)                                                     
 *
 */
enum {
     /* The lowest layer, used for HUD-like displays while flying.                  */
     xplm_WindowLayerFlightOverlay            = 0

     /* Windows that "float" over the sim, like the X-Plane 11 map does. If you are *
      * not sure which layer to create your window in, choose floating.             */
    ,xplm_WindowLayerFloatingWindows          = 1

     /* An interruptive modal that covers the sim with a transparent black overlay  *
      * to draw the user's focus to the alert                                       */
    ,xplm_WindowLayerModal                    = 2

     /* "Growl"-style notifications that are visible in a corner of the screen,     *
      * even over modals                                                            */
    ,xplm_WindowLayerGrowlNotifications       = 3


};
typedef int XPLMWindowLayer;
#endif /* XPLM300 */

#if defined(XPLM200)
/*
 * XPLMCreateWindow_t
 * 
 * The XPMCreateWindow_t structure defines all of the parameters used to 
 * create a modern window using XPLMCreateWindowEx().  The structure will be 
 * expanded in future SDK APIs to include more features.  Always set the 
 * structSize member to the size of your struct in bytes! 
 * 
 * All windows created by this function in the XPLM300 version of the API are 
 * created with the new X-Plane 11 GUI features. This means your plugin will 
 * get to "know" about the existence of X-Plane windows other than the main 
 * window. All drawing and mouse callbacks for your window will occur in 
 * "boxels," giving your windows automatic support for high-DPI scaling in 
 * X-Plane. In addition, your windows can opt-in to decoration with the 
 * X-Plane 11 window styling, and you can use the 
 * XPLMSetWindowPositioningMode() API to make your window "popped out" into a 
 * first-class operating system window. 
 * 
 * Note that this requires dealing with your window's bounds in "global 
 * desktop" positioning units, rather than the traditional panel coordinate 
 * system. In global desktop coordinates, the main X-Plane window may not have 
 * its origin at coordinate (0,0), and your own window may have negative 
 * coordinates. Assuming you don't implicitly assume (0, 0) as your origin, 
 * the only API change you should need is to start using 
 * XPLMGetMouseLocationGlobal() rather than XPLMGetMouseLocation(), and 
 * XPLMGetScreenBoundsGlobal() instead of XPLMGetScreenSize(). 
 * 
 * If you ask to be decorated as a floating window, you'll get the blue window 
 * control bar and blue backing that you see in X-Plane 11's normal "floating" 
 * windows (like the map).                                                     
 *
 */
typedef struct {
     /* Used to inform XPLMCreateWindowEx() of the SDK version you compiled         *
      * against; should always be set to sizeof(XPLMCreateWindow_t)                 */
     int                       structSize;
     /* Left bound, in global desktop boxels                                        */
     int                       left;
     /* Top bound, in global desktop boxels                                         */
     int                       top;
     /* Right bound, in global desktop boxels                                       */
     int                       right;
     /* Bottom bound, in global desktop boxels                                      */
     int                       bottom;
     int                       visible;
     XPLMDrawWindow_f          drawWindowFunc;
     /* A callback to handle the user left-clicking within your window (or NULL to  *
      * ignore left clicks)                                                         */
     XPLMHandleMouseClick_f    handleMouseClickFunc;
     XPLMHandleKey_f           handleKeyFunc;
     XPLMHandleCursor_f        handleCursorFunc;
     XPLMHandleMouseWheel_f    handleMouseWheelFunc;
     void *                    refcon;
#if defined(XPLM300)
     /* True if you want to have an X-Plane 11-style "wrapper" around your window   *
      * (like the X-Plane 11 map)                                                   */
     int                       decorateAsFloatingWindow;
#endif /* XPLM300 */
#if defined(XPLM300)
     XPLMWindowLayer           layer;
#endif /* XPLM300 */
#if defined(XPLM300)
     /* A callback to handle the user right-clicking within your window (or NULL to *
      * ignore right clicks)                                                        */
     XPLMHandleMouseClick_f    handleRightClickFunc;
#endif /* XPLM300 */
} XPLMCreateWindow_t;
#endif /* XPLM200 */

#if defined(XPLM200)
/*
 * XPLMCreateWindowEx
 * 
 * This routine creates a new "modern" window. You pass in an 
 * XPLMCreateWindow_t structure with all of the fields set in.  You must set 
 * the structSize of the structure to the size of the actual structure you 
 * used.  Also, you must provide functions for every callback---you may not 
 * leave them null!  (If you do not support the cursor or mouse wheel, use 
 * functions that return the default values.)  The numeric values of the 
 * XPMCreateWindow_t structure correspond to the parameters of 
 * XPLMCreateWindow.                                                           
 *
 */
XPLM_API XPLMWindowID         XPLMCreateWindowEx(
                                   XPLMCreateWindow_t * inParams);    
#endif /* XPLM200 */

/*
 * XPLMCreateWindow
 * 
 * Deprecated as of XPLM300. 
 * 
 * This routine creates a new legacy window. Unlike modern windows (created 
 * via XPLMCreateWindowEx()), legacy windows do not have access to X-Plane 11 
 * features like automatic scaling for high-DPI screens, native window styles, 
 * or support for being "popped out" into first-class operating system 
 * windows. 
 * 
 * Pass in the dimensions and offsets to the window's bottom left corner from 
 * the bottom left of the screen.  You can specify whether the window is 
 * initially visible or not.  Also, you pass in three callbacks to run the 
 * window and a refcon.  This function returns a window ID you can use to 
 * refer to the new window. 
 * 
 * NOTE: Legacy windows do not have "frames"; you are responsible for drawing 
 * the background and frame of the window.  Higher level libraries have 
 * routines which make this easy.                                              
 *
 */
XPLM_API XPLMWindowID         XPLMCreateWindow(
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom,    
                                   int                  inIsVisible,    
                                   XPLMDrawWindow_f     inDrawCallback,    
                                   XPLMHandleKey_f      inKeyCallback,    
                                   XPLMHandleMouseClick_f inMouseCallback,    
                                   void *               inRefcon);    

/*
 * XPLMDestroyWindow
 * 
 * This routine destroys a window.  The window's callbacks are not called 
 * after this call. Keyboard focus is removed from the window before 
 * destroying it.                                                              
 *
 */
XPLM_API void                 XPLMDestroyWindow(
                                   XPLMWindowID         inWindowID);    

/*
 * XPLMGetScreenSize
 * 
 * This routine returns the size of the X-Plane OpenGL window in pixels.  
 * Please note that this is not the size of the screen when  doing 2-d drawing 
 * (the 2-d screen is currently always 1024x768, and  graphics are scaled up 
 * by OpenGL when doing 2-d drawing for higher-res monitors).  This number can 
 * be used to get a rough idea of the amount of detail the user will be able 
 * to see when drawing in 3-d.                                                 
 *
 */
XPLM_API void                 XPLMGetScreenSize(
                                   int *                outWidth,    /* Can be NULL */
                                   int *                outHeight);    /* Can be NULL */

#if defined(XPLM300)
/*
 * XPLMGetScreenBoundsGlobal
 * 
 * This routine returns the bounds of the "global" X-Plane desktop, in boxels. 
 * Unlike the non-global version XPLMGetScreenSize(), this is multi-monitor 
 * aware. There are three primary consequences of multimonitor awareness. 
 * 
 * First, if the user is running X-Plane in full-screen on two or more 
 * monitors (typically configured using one full-screen window per monitor), 
 * the global desktop will be sized to include all X-Plane windows. 
 * 
 * Second, the origin of the screen coordinates is not guaranteed to be (0, 
 * 0). Suppose the user has two displays side-by-side, both running at 1080p. 
 * Suppose further that they've configured their OS to make the left display 
 * their "primary" monitor, and that X-Plane is running in full-screen on 
 * their right monitor. In this case, the global desktop bounds would be the 
 * rectangle from (1920, 0) to (3840, 1080). If the user later asked X-Plane 
 * to draw on their primary monitor as well, the bounds would change to (0, 0) 
 * to (3840, 1080). 
 * 
 * Finally, if the usable area of the virtual desktop is not a perfect 
 * rectangle (for instance, because the monitors have different resolutions or 
 * because one monitor is configured in the operating system to be above and 
 * to the right of the other), the global desktop will include any wasted 
 * space. Thus, if you have two 1080p monitors, and monitor 2 is configured to 
 * have its bottom left touch monitor 1's upper right, your global desktop 
 * area would be the rectangle from (0, 0) to (3840, 2160). 
 * 
 * Note that popped-out windows (windows drawn in their own operating system 
 * windows, rather than "floating" within X-Plane) are not included in these 
 * bounds.                                                                     
 *
 */
XPLM_API void                 XPLMGetScreenBoundsGlobal(
                                   int *                outLeft,    /* Can be NULL */
                                   int *                outTop,    /* Can be NULL */
                                   int *                outRight,    /* Can be NULL */
                                   int *                outBottom);    /* Can be NULL */
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMGetAllMonitorBoundsGlobal
 * 
 * This routine immediately calls you back with the bounds (in boxels) of each 
 * full-screen X-Plane window within the X-Plane global desktop space. (Note 
 * that if a monitor is *not* covered by an X-Plane window, you cannot get its 
 * bounds this way. Likewise, monitors with only an X-Plane window will not be 
 * included.) 
 * 
 * If X-Plane is running in full-screen, then the combined global bounds of 
 * all full-screen monitors will match the total global desktop bounds, as 
 * returned by XPLMGetScreenBoundsGlobal(). (Of course, if X-Plane is running 
 * in windowed mode, this will not be the case.) 
 * 
 * Note that this function's monitor indices match those provided by 
 * XPLMGetAllMonitorBoundsOS(), but the coordinates are different (since the 
 * X-Plane global desktop may not match the operating system's global desktop, 
 * and one X-Plane boxel may be larger than one pixel).                        
 *
 */
XPLM_API void                 XPLMGetAllMonitorBoundsGlobal(
                                   XPLMReceiveMonitorBoundsGlobal_f inMonitorBoundsCallback,    
                                   void *               refcon);    
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMGetAllMonitorBoundsOS
 * 
 * This routine immediately calls you back with the bounds (in pixels) of each 
 * monitor within the operating system's global desktop space. Note that 
 * unlike XPLMGetAllMonitorBoundsGlobal(), this may include monitors that have 
 * no X-Plane window on them. 
 * 
 * Note that this function's monitor indices match those provided by 
 * XPLMGetAllMonitorBoundsGlobal(), but the coordinates are different (since 
 * the X-Plane global desktop may not match the operating system's global 
 * desktop, and one X-Plane boxel may be larger than one pixel).               
 *
 */
XPLM_API void                 XPLMGetAllMonitorBoundsOS(
                                   XPLMReceiveMonitorBoundsOS_f inMonitorBoundsCallback,    
                                   void *               refcon);    
#endif /* XPLM300 */

/*
 * XPLMGetMouseLocation
 * 
 * Deprecated in XPLM300. Modern windows should use 
 * XPLMGetMouseLocationGlobal() instead. 
 * 
 * This routine returns the current mouse location in cockpit pixels.  The 
 * bottom left corner of the display is 0,0.  Pass NULL to not receive info 
 * about either parameter. 
 * 
 * Because this function gives the mouse position relative to the main X-Plane 
 * window (rather than in global bounds), this function should only be used by 
 * legacy windows. Modern windows should instead get the mouse position in 
 * global desktop coordinates using XPLMGetMouseLocationGlobal(). 
 * 
 * Note that unlike XPLMGetMouseLocationGlobal(), if the mouse goes outside 
 * the user's main monitor (for instance, to a pop out window or a secondary 
 * monitor), this function will not reflect it.                                
 *
 */
XPLM_API void                 XPLMGetMouseLocation(
                                   int *                outX,    /* Can be NULL */
                                   int *                outY);    /* Can be NULL */

#if defined(XPLM300)
/*
 * XPLMGetMouseLocationGlobal
 * 
 * Returns the current mouse location in global desktop boxels. Unlike 
 * XPLMGetMouseLocation(), the bottom left of the display is not guaranteed to 
 * be 0,0---instead, the origin is the lower left of the entire global desktop 
 * space. In addition, this routine gives the real mouse location when the 
 * mouse goes to X-Plane windows other than the primary display. Thus, it can 
 * be used with both pop-out windows and secondary monitors. 
 * 
 * This is the mouse location function to use with modern windows (i.e., those 
 * created by XPLMCreateWindowEx()). 
 * 
 * Pass NULL to not receive info about either parameter.                       
 *
 */
XPLM_API void                 XPLMGetMouseLocationGlobal(
                                   int *                outX,    /* Can be NULL */
                                   int *                outY);    /* Can be NULL */
#endif /* XPLM300 */

/*
 * XPLMGetWindowGeometry
 * 
 * This routine returns the position and size of a window. The units and 
 * coordinate system vary depending on the type of window you have. 
 * 
 * If this is a legacy window (one compiled against a pre-XPLM300 version of 
 * the SDK, or an XPLM300 window that was not created using 
 * XPLMCreateWindowEx()), the units are cockpit pixels. 
 * 
 * If, on the other hand, this is a new X-Plane 11-style window (compiled 
 * against the XPLM300 SDK and created using XPLMCreateWindowEx()), the units 
 * are global desktop boxels. 
 * 
 * Pass NULL to not receive any paramter.                                      
 *
 */
XPLM_API void                 XPLMGetWindowGeometry(
                                   XPLMWindowID         inWindowID,    
                                   int *                outLeft,    /* Can be NULL */
                                   int *                outTop,    /* Can be NULL */
                                   int *                outRight,    /* Can be NULL */
                                   int *                outBottom);    /* Can be NULL */

/*
 * XPLMSetWindowGeometry
 * 
 * This routine allows you to set the position and size of a window. 
 * 
 * The units and coordinate system match those of XPLMGetWindowGeometry(). 
 * That is, modern windows use global desktop coordinates, while legacy 
 * windows use cockpit pixels. 
 * 
 * Note that this only applies to "floating" windows (that is, windows that 
 * are drawn within the X-Plane simulation windows, rather than being "popped 
 * out" into their own first-class operating system windows). To set the 
 * position of windows whose positioning mode is xplm_WindowPopOut, you'll 
 * need to instead use XPLMSetWindowGeometryOS().                              
 *
 */
XPLM_API void                 XPLMSetWindowGeometry(
                                   XPLMWindowID         inWindowID,    
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom);    

#if defined(XPLM300)
/*
 * XPLMGetWindowGeometryOS
 * 
 * This routine returns the position and size of a "popped out" window (i.e., 
 * a window whose positioning mode is xplm_WindowPopOut), in operating system 
 * pixels.  Pass NULL to not receive any parameter.                            
 *
 */
XPLM_API void                 XPLMGetWindowGeometryOS(
                                   XPLMWindowID         inWindowID,    
                                   int *                outLeft,    /* Can be NULL */
                                   int *                outTop,    /* Can be NULL */
                                   int *                outRight,    /* Can be NULL */
                                   int *                outBottom);    /* Can be NULL */
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMSetWindowGeometryOS
 * 
 * This routine allows you to set the position and size, in operating system 
 * pixel coordinates, of a popped out window (that is, a window whose 
 * positioning mode is xplm_WindowPopOut, which exists outside the X-Plane 
 * simulation window, in its own first-class operating system window). Note 
 * that you are responsible for ensuring both that your window is popped out 
 * (using XPLMWindowIsPoppedOut()) and that a monitor really exists at the OS 
 * coordinates you provide (using XPLMGetAllMonitorBoundsOS()).                
 *
 */
XPLM_API void                 XPLMSetWindowGeometryOS(
                                   XPLMWindowID         inWindowID,    
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom);    
#endif /* XPLM300 */

/*
 * XPLMGetWindowIsVisible
 * 
 * This routine returns whether a window is visible.                           
 *
 */
XPLM_API int                  XPLMGetWindowIsVisible(
                                   XPLMWindowID         inWindowID);    

/*
 * XPLMSetWindowIsVisible
 * 
 * This routine shows or hides a window.                                       
 *
 */
XPLM_API void                 XPLMSetWindowIsVisible(
                                   XPLMWindowID         inWindowID,    
                                   int                  inIsVisible);    

#if defined(XPLM300)
/*
 * XPLMWindowIsPoppedOut
 * 
 * True if this window has been popped out (making it a first-class window in 
 * the operating system), which in turn is true if and only if you have set 
 * the window's positioning mode to xplm_WindowPopOut.                         
 *
 */
XPLM_API int                  XPLMWindowIsPoppedOut(
                                   XPLMWindowID         inWindowID);    
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMSetWindowGravity
 * 
 * A window's "gravity" controls how the window shifts as the whole X-Plane 
 * window resizes. A gravity of 1 means the window maintains its positioning 
 * relative to the right or top edges, 0 the left/bottom, and 0.5 keeps it 
 * centered. 
 * 
 * Default gravity is (0, 1, 0, 1), meaning your window will maintain its 
 * position relative to the top left and will not change size as its 
 * containing window grows. 
 * 
 * If you wanted, say, a window that sticks to the top of the screen (with a 
 * constant height), but which grows to take the full width of the window, you 
 * would pass (0, 1, 1, 1). Because your left and right edges would maintain 
 * their positioning relative to their respective edges of the screen, the 
 * whole width of your window would change with the X-Plane window. 
 * 
 * Note that this only applies to modern windows. (Windows created using the 
 * deprecated XPLMCreateWindow(), or windows compiled against a pre-XPLM300 
 * version of the SDK will simply get the default gravity.)                    
 *
 */
XPLM_API void                 XPLMSetWindowGravity(
                                   XPLMWindowID         inWindowID,    
                                   float                inLeftGravity,    
                                   float                inTopGravity,    
                                   float                inRightGravity,    
                                   float                inBottomGravity);    
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMSetWindowResizingLimits
 * 
 * Sets the minimum and maximum size of the client rectangle of the given 
 * window. (That is, it does not include any window styling that you might 
 * have asked X-Plane to apply on your behalf.) All resizing operations are 
 * constrained to these sizes. 
 * 
 * Only applies to modern windows. (Windows created using the deprecated 
 * XPLMCreateWindow(), or windows compiled against a pre-XPLM300 version of 
 * the SDK will have no minimum or maximum size.)                              
 *
 */
XPLM_API void                 XPLMSetWindowResizingLimits(
                                   XPLMWindowID         inWindowID,    
                                   int                  inMinWidthBoxels,    
                                   int                  inMinHeightBoxels,    
                                   int                  inMaxWidthBoxels,    
                                   int                  inMaxHeightBoxels);    
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMWindowPositioningMode
 * 
 * XPLMWindowPositionMode describes how X-Plane will position your window on 
 * the user's screen. X-Plane will maintain this positioning mode even as the 
 * user resizes their window or adds/removes full-screen monitors. 
 * 
 * Positioning mode can only be set for "modern" windows (that is, windows 
 * created using XPLMCreateWindowEx() and compiled against the XPLM300 SDK). 
 * Windows created using the deprecated XPLMCreateWindow(), or windows 
 * compiled against a pre-XPLM300 version of the SDK will simply get the 
 * "free" positioning mode.                                                    
 *
 */
enum {
     /* The default positioning mode. Set the window geometry and its future        *
      * position will be determined by its window gravity, resizing limits, and     *
      * user interactions.                                                          */
     xplm_WindowPositionFree                  = 0

     /* Keep the window centered on the monitor you specify                         */
    ,xplm_WindowCenterOnMonitor               = 1

     /* Keep the window full screen on the monitor you specify                      */
    ,xplm_WindowFullScreenOnMonitor           = 2

     /* Like gui_window_full_screen_on_monitor, but stretches over *all* monitors   *
      * and popout windows. This is an obscure one... unless you have a very good   *
      * reason to need it, you probably don't!                                      */
    ,xplm_WindowFullScreenOnAllMonitors       = 3

     /* A first-class window in the operating system, completely separate from the  *
      * X-Plane window(s)                                                           */
    ,xplm_WindowPopOut                        = 4


};
typedef int XPLMWindowPositioningMode;
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMSetWindowPositioningMode
 * 
 * Sets the policy for how X-Plane will position your window. 
 * 
 * Some positioning modes apply to a particular monitor. For those modes, you 
 * can pass a negative monitor index to position the window on the main 
 * X-Plane monitor (the screen with the X-Plane menu bar at the top). Or, if 
 * you have a specific monitor you want to position your window on, you can 
 * pass a real monitor index as received from, e.g., 
 * XPLMGetAllMonitorBoundsOS(). 
 * 
 * Only applies to modern windows. (Windows created using the deprecated 
 * XPLMCreateWindow(), or windows compiled against a pre-XPLM300 version of 
 * the SDK will always use xplm_WindowPositionFree.)                           
 *
 */
XPLM_API void                 XPLMSetWindowPositioningMode(
                                   XPLMWindowID         inWindowID,    
                                   XPLMWindowPositioningMode inPositioningMode,    
                                   int                  inMonitorIndex);    
#endif /* XPLM300 */

#if defined(XPLM300)
/*
 * XPLMSetWindowTitle
 * 
 * Sets the name for a window. This only applies to windows that opted-in to 
 * styling as an X-Plane 11 floating window when they were created using 
 * XPLMCreateWindowEx().                                                       
 *
 */
XPLM_API void                 XPLMSetWindowTitle(
                                   XPLMWindowID         inWindowID,    
                                   const char *         inWindowTitle);    
#endif /* XPLM300 */

/*
 * XPLMGetWindowRefCon
 * 
 * This routine returns a window's reference constant, the unique value you 
 * can use for your own purposes.                                              
 *
 */
XPLM_API void *               XPLMGetWindowRefCon(
                                   XPLMWindowID         inWindowID);    

/*
 * XPLMSetWindowRefCon
 * 
 * This routine sets a window's reference constant.  Use this to pass data to 
 * yourself in the callbacks.                                                  
 *
 */
XPLM_API void                 XPLMSetWindowRefCon(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon);    

/*
 * XPLMTakeKeyboardFocus
 * 
 * This routine gives a specific window keyboard focus.  Keystrokes will be 
 * sent to  that window.  Pass a window ID of 0 to pass keyboard strokes 
 * directly to X-Plane.                                                        
 *
 */
XPLM_API void                 XPLMTakeKeyboardFocus(
                                   XPLMWindowID         inWindow);    

/*
 * XPLMBringWindowToFront
 * 
 * This routine brings the window to the front of the Z-order.  Windows are 
 * brought to the front when they are created. Beyond that, you should make 
 * sure you are front before handling mouse clicks. 
 * 
 * Note that this only brings your window to the front of its layer 
 * (XPLMWindowLayer). Thus, if you have a window in the floating window layer 
 * (xplm_WindowLayerFloatingWindows), but there is a modal window (in layer 
 * xplm_WindowLayerModal) above you, you would still not be the true frontmost 
 * window after calling this. (After all, the window layers are strictly 
 * ordered, and no window in a lower layer can ever be above any window in a 
 * higher one.)                                                                
 *
 */
XPLM_API void                 XPLMBringWindowToFront(
                                   XPLMWindowID         inWindow);    

/*
 * XPLMIsWindowInFront
 * 
 * This routine returns true if the window you passed in is the frontmost 
 * visible window in its layer (XPLMWindowLayer). 
 * 
 * Thus, if you have a window at the front of the floating window layer 
 * (xplm_WindowLayerFloatingWindows), this will return true even if there is a 
 * modal window (in layer xplm_WindowLayerModal) above you. (Not to worry, 
 * though: in such a case, X-Plane will not pass clicks or keyboard input down 
 * to your layer until the window above stops "eating" the input.)             
 *
 */
XPLM_API int                  XPLMIsWindowInFront(
                                   XPLMWindowID         inWindow);    

/***************************************************************************
 * HOT KEYS
 ***************************************************************************/
/*
 * Hot Keys - keystrokes that can be managed by others.                        
 *
 */


/*
 * XPLMHotKey_f
 * 
 * Your hot key callback simply takes a pointer of your choosing.              
 *
 */
typedef void (* XPLMHotKey_f)(
                                   void *               inRefcon);    

/*
 * XPLMHotKeyID
 * 
 * Hot keys are identified by opaque IDs.                                      
 *
 */
typedef void * XPLMHotKeyID;

/*
 * XPLMRegisterHotKey
 * 
 * This routine registers a hot key.  You specify your preferred key stroke 
 * virtual key/flag combination, a description of what your callback does (so 
 * other plug-ins can describe the plug-in to the user for remapping) and a 
 * callback function and opaque pointer to pass in).  A new hot key ID is 
 * returned.  During execution, the actual key associated with your hot key 
 * may change, but you are insulated from this.                                
 *
 */
XPLM_API XPLMHotKeyID         XPLMRegisterHotKey(
                                   char                 inVirtualKey,    
                                   XPLMKeyFlags         inFlags,    
                                   const char *         inDescription,    
                                   XPLMHotKey_f         inCallback,    
                                   void *               inRefcon);    

/*
 * XPLMUnregisterHotKey
 * 
 * This API unregisters a hot key.  You can only register your own hot keys.   
 *
 */
XPLM_API void                 XPLMUnregisterHotKey(
                                   XPLMHotKeyID         inHotKey);    

/*
 * XPLMCountHotKeys
 * 
 * Returns the number of current hot keys.                                     
 *
 */
XPLM_API int                  XPLMCountHotKeys(void);

/*
 * XPLMGetNthHotKey
 * 
 * Returns a hot key by index, for iteration on all hot keys.                  
 *
 */
XPLM_API XPLMHotKeyID         XPLMGetNthHotKey(
                                   int                  inIndex);    

/*
 * XPLMGetHotKeyInfo
 * 
 * Returns information about the hot key.  Return NULL for any  parameter you 
 * don't want info about.  The description should be at least 512 chars long.  
 *
 */
XPLM_API void                 XPLMGetHotKeyInfo(
                                   XPLMHotKeyID         inHotKey,    
                                   char *               outVirtualKey,    /* Can be NULL */
                                   XPLMKeyFlags *       outFlags,    /* Can be NULL */
                                   char *               outDescription,    /* Can be NULL */
                                   XPLMPluginID *       outPlugin);    /* Can be NULL */

/*
 * XPLMSetHotKeyCombination
 * 
 * XPLMSetHotKeyCombination remaps a hot keys keystrokes.  You may remap 
 * another plugin's keystrokes.                                                
 *
 */
XPLM_API void                 XPLMSetHotKeyCombination(
                                   XPLMHotKeyID         inHotKey,    
                                   char                 inVirtualKey,    
                                   XPLMKeyFlags         inFlags);    

#ifdef __cplusplus
}
#endif

#endif