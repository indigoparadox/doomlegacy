/* 
 Copyright (C) 2011 by DooM Legacy Team.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
*/
/* [WDJ] 12/5/2011 Customized from SDL demos.
Has been written for DoomLegacy as much as possible considering that
most everything in this file is forced by the Mac interface.  Almost any
variation from the examples will cause compilation failure or execution failure.
Kept same names when unsure of external references.

Need NSAutoreleasePool.
DoomLegacy does not use apple menus, nor other features. 
No NIB (not frameworks), no CPS (docking).
Do not change current directory.  DoomLegacy uses current directory when run
from command line or script, and has its own dir search.
*/

//#define SDL_CHANGE_DIRECTORY  1
#define PROGRAM_NAME   "DoomLegacy"

/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#if 1
#import <SDL.h>
#else
#import <SDL/SDL.h>
#endif
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

/* SDLmain header from SDL demos */
#import <Cocoa/Cocoa.h>
@interface SDLMain : NSObject
@end
/* end header */

/* Testing whether we need to use SDLMain.nib or not */
#define  SDL_USE_NIB_FILE   0

/* Testing whether we need to need to use CPS (docking) or not */
#if 1
/* DEBUG: Test if CPS is needed */
#define  SDL_USE_CPS   1
#else
//#define  SDL_USE_CPS   0
#endif

#ifdef SDL_USE_CPS
/* Portions of CPS.h */
typedef struct CPSProcessSerNum
{
    UInt32  lo;
    UInt32  hi;
} CPSProcessSerNum;

extern OSErr  CPSGetCurrentProcess( CPSProcessSerNum *psn);
extern OSErr  CPSEnableForegroundOperation( CPSProcessSerNum *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
extern OSErr  CPSSetFrontProcess( CPSProcessSerNum *psn);

#endif /* SDL_USE_CPS */
#endif


// global arguments (do not know if names are important so same as everybody else)
static int   gArgc;
static char  **gArgv;
static BOOL  gFinderLaunch;

#ifdef PROGRAM_NAME
NSString * program_app_name = @ PROGRAM_NAME;
#else
static NSString * get_program_name( void )
{
    NSString * progname = 0;  // program name
    NSDictionary * dict;

    /* get the application name from the process */
    dict = (NSDictionary *)CFBundleGetInfoDictionary( CFBundleGetMainBundle() );
    if (dict)
       progname = [dict objectForKey: @"CFBundleName"];
    
    if (![progname length])
       progname = [[NSProcessInfo processInfo] processName];

    return progname;
}


#ifdef SDL_USE_NIB_FILE
/* helper category for NSString */
@interface NSString (ReplaceSubString)
- (NSString *) replace_in_string :(NSRange)str_range with:(NSString *)strin;
@end

  
@implementation NSString (ReplaceSubString)

- (NSString *) replace_in_string :(NSRange)str_range with:(NSString *)strin
{
    NSString * strout;
    NSRange rep_range;
    unsigned int self_len = [self length];
    unsigned int strin_len = [strin length];
    unsigned int buffer_size;
    unichar * buffer;

    /* replace the characters in range with strin */
    buffer_size = self_len + strin_len - str_range.length;
    buffer = NSAllocateMemoryPages( buffer_size * sizeof(unichar) );
    
    /* copy before range into buffer */
    rep_range.location = 0;
    rep_range.length = str_range.location;
    [self getCharacters:buffer range:rep_range];
    
    /* copy strin */
    rep_range.location = 0;
    rep_range.length = strin_len;
    [strin getCharacters:(buffer + str_range.location) range:rep_range];
     
    /* copy after range into buffer */
    rep_range.location = str_range.location + str_range.length;
    rep_range.length = self_len - rep_range.location;
    [self getCharacters:(buffer+str_range.location+strin_len) range:rep_range];
    
    /* make a string from buffer */
    strout = [NSString stringWithCharacters:buffer length:buffer_size];
    
    NSDeallocateMemoryPages(buffer, buffer_size);
    
    return strout;
}

@end

#else
/* From Edge-1.35: For some reason, Apple removed setAppleMenu from the 
headers in 10.4, but the method still is there and works. 
To avoid warnings, declare it here. */
@interface NSApplication(SDL_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end
#endif

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event  event;
    event.type = SDL_QUIT;
    SDL_PushEvent( &event );
}
@end

/* The main class of the application, the application's delegate */
@implementation SDLMain

#ifdef SDL_CHANGE_DIRECTORY
/* Set working directory to the app parent directory */
- (void) set_work_dir:(BOOL)enable_chdir
{
    if (enable_chdir)
    {
        char parentdir[MAXPATHLEN];
        CFURLRef url1 = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
        CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url1);
        if (CFURLGetFileSystemRepresentation(url2, true, parentdir, MAXPATHLEN)) {
	   assert ( chdir (parentdir) == 0 );   /* chdir to the binary app's parent */
	}
        CFRelease( url2 );
        CFRelease( url1 );
    }
}
#endif

#ifdef SDL_USE_NIB_FILE

/* Change menu to new_name */
- (void) change_menu_name:(NSMenu *)chmenu to:(NSString *)new_app_name
{
    NSRange str_range;
    NSEnumerator * menu_enumtor;
    NSMenuItem * menu_item;

    /* find placeholder name "SDL App" */
    str_range = [[chmenu title] rangeOfString:@"SDL App"];
    if (str_range.length != 0)
        [chmenu setTitle: [[chmenu title] replace_in_string:str_range with:new_app_name]];

    menu_enumtor = [[chmenu itemArray] objectEnumerator];
    while ((menu_item = [menu_enumtor nextObject]))
    {
        str_range = [[menu_item title] rangeOfString:@"SDL App"];
        if (str_range.length != 0)
            [menu_item setTitle: [[menu_item title] replace_in_string:str_range with:new_app_name]];
        if ([menu_item hasSubmenu])
            [self change_menu_name:[menu_item submenu] to:new_app_name];
    }
    [ chmenu sizeToFit ];
}

#else

static void create_doomlegacy_mac_menus ( void )
{
    /* warning: this code is very odd */
    NSMenu * apple_menu;
    NSMenu * window_menu;
    NSMenuItem * menu_item;
    NSString * title;
    NSString * appname;

#ifdef PROGRAM_NAME    
    appname = program_app_name;
#else
    appname = get_program_name();
#endif

    /* Program/App menu */
    apple_menu = [[NSMenu alloc] initWithTitle:@""];

    /* standard menu items */
//    title = [@"About " stringByAppendingString:appname];
//    [apple_menu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

//    [apple_menu addItem:[NSMenuItem separatorItem]];

    title = [@"Hide " stringByAppendingString:appname];
    [apple_menu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

    menu_item = (NSMenuItem *)[apple_menu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menu_item setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];

    [apple_menu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

    [apple_menu addItem:[NSMenuItem separatorItem]];

    title = [@"Quit " stringByAppendingString:appname];
    [apple_menu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
    
    /* Insert this menu into the window menubar */
    menu_item = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [menu_item setSubmenu:apple_menu];
    [[NSApp mainMenu] addItem:menu_item];
    [menu_item release];

    /* Set this menu as the application menu */
    [NSApp setAppleMenu:apple_menu];
    [apple_menu release];

    /* Window menu */
    window_menu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    /* Standard menu items */
    menu_item = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [window_menu addItem:menu_item];
    [menu_item release];
    
    /* Insert this menu into the menubar */
    menu_item = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [menu_item setSubmenu:window_menu];
    [[NSApp mainMenu] addItem:menu_item];
    [menu_item release];
    
    /* Set the new menu as the application menu */
    [NSApp setWindowsMenu:window_menu];
    [window_menu release];
}

/* Replacement for NSApplicationMain */
static void DL_app_main (int argc, char ** argv)
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    SDLMain * sdl_main_prog;

    /* Initialize (or reinit) the program application */
    [SDLApplication sharedApplication];
    
#ifdef SDL_USE_CPS
    {
        // whatever CPS is, it is optional
        CPSProcessSerNum PSN;
        /* info on this process is sent to the dock */
        if (!CPSGetCurrentProcess(&PSN))
            if (!CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103))
                if (!CPSSetFrontProcess(&PSN))
                    [SDLApplication sharedApplication];
    }
#endif /* SDL_USE_CPS */

    /* Set up the menubar */
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    create_doomlegacy_mac_menus();

    /* Create SDLMain process and set it as the app delegate ?? */
    sdl_main_prog = [[SDLMain alloc] init];
    [NSApp setDelegate:sdl_main_prog];
    
    /* Run it, (Start the main event loop??) */
    [NSApp run];
    
    /* Release local obj references */
    [sdl_main_prog release];
    [pool release];
}

#endif

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

#ifdef SDL_CHANGE_DIRECTORY
    /* Set working directory to the app parent directory */
    [self set_work_dir:gFinderLaunch];
#endif

#ifdef SDL_USE_NIB_FILE
    /* Change the main menu to have the program name */
# ifdef PROGRAM_NAME    
    [self change_menu_name:[NSApp mainMenu] to:program_app_name];
# else
    [self change_menu_name:[NSApp mainMenu] to:get_program_name()];
# endif
#endif

    /* Run SDL_main in the DoomLegacy program */
    status = SDL_main (gArgc, gArgv);

    /* Exit the program */
    exit(status);
}
@end



#ifdef main
#  undef main
#endif


/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
    /* Copy the arguments into a global variable */
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        gArgc = argc;
        gFinderLaunch = NO;
    }
    gArgv = argv;

#ifdef SDL_USE_NIB_FILE
    [SDLApplication poseAsClass:[NSApplication class]];
    NSApplicationMain (argc, argv);
#else
    DL_app_main (argc, argv);
#endif
    return 0;
}
