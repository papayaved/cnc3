#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#define APP_VER ("3.0.0.22b")
/*
    0.2.2.1 изменен информационный пакет ЧПУ
    0.2.2.2 попытка перевернуть оси в QWT
    0.2.2.3 исправление ошибок Idle Run, Circle generator, Options->Write
    0.2.2.4 debug feedback
    0.2.2.8 Command keys in Code Editor, clear Run panel
    0.2.2.0 Time AM PM for non-russian languages
    0.3.0.0 Slow speed, write default settings on the disk if settings are empty
    0.3.0.1 Fixed an error step and scale
    0.3.0.2 Fixed an arc-to-arc intersection, enabled Default button on the pass window
    0.3.0.3 Edited English help, controls texts, set help focus when it opens, changed translates, add memtest
    0.3.0.4 Open and save the project and passes parameters. Corrected slicing (offset = 0)
    0.3.0.6 Corrected long loading for Linux computer. Commented flush() (don't work) and QMutexLocker (work slow)
    0.3.0.7 Error when clicking on the generate button. Try to catch it. Fixed a double click
    0.3.0.8 Fixed default dir path to /home/cnc
    0.3.0.9 Program do several attempts to connect and after this return to Home panel and reconnect. Added WDT analizing
    0.3.0.10 Fixed Reverse Encoder settings
    0.3.0.11 Added cnc_state_reset() before Start
    3.0.0.12 Corrected the remaining time for the angular cutting (todo more)
    3.0.0.13 Added external permit signal for a Stone version
    3.0.0.14 Added for slicing an idle run
    3.0.0.15 Problem with backup
    3.0.0.16 Disabled a context menu in the Help
    3.0.0.17 Dark theme (test)
    3.0.0.18 Changed MCU project (Soft pult abort at alarm and wire break). Moved pult controls
    3.0.0.19 Generator wizards changed (added pause, save values, Undo)
    3.0.0.20 changed M codes, pass panel, modes table
    3.0.0.21 updated help, supported DXF lwpolyline, layers, enable dark theme for plot only, rename "drum" to "roll", work with UV in countor editor
    3.0.0.22 Update the Contour editor, added contour multiplexor, select contours on plot, move segments
*/

//#define STONE
//#define TEST_PANEL
#ifndef STONE
    #define SHOW_ENCODERS
#endif
//#define TEST_ADC

#ifdef LINUX // - defined in pro file
#define DEFAULT_COM ("/dev/ttyACM0")
#else
#define DEFAULT_COM ("COM6")
#endif

#define ST_VENDOR_ID (0x483)
#define ST_PRODUCT_ID (0x5740)

#define FULL_SCREEN

//#define DARK_GUI_THEME
#define DARK_PLOT_THEME

#endif // CONSTRAINTS_H
