/**
 * @file lv_ex_conf.h
 *
 */

#ifndef LV_EX_CONF_H
#define LV_EX_CONF_H


/*******************
 * GENERAL SETTING
 *******************/
#if USE_EVDEV
#define LV_EX_PRINTF       0
#define LV_EX_KEYBOARD     0
#define LV_EX_MOUSEWHEEL   0
#else
#define LV_EX_PRINTF       1       /*Enable printf-ing data*/
#define LV_EX_KEYBOARD     1       /*Add PC keyboard support to some examples (`lv_drvers` repository is required)*/
#define LV_EX_MOUSEWHEEL   1       /*Add 'encoder' (mouse wheel) support to some examples (`lv_drivers` repository is required)*/
#endif  /*USE_EVDEV*/

/*******************
 *   TEST USAGE
 *******************/
#if USE_EVDEV
#define LV_USE_TESTS       0
#else
#define LV_USE_TESTS       1
#endif  /*USE_EVDEV*/

/*******************
 * TUTORIAL USAGE
 *******************/
#define LV_USE_TUTORIALS   1

/*********************
 * APPLICATION USAGE
 *********************/

/* Test the graphical performance of your MCU
 * with different settings*/
#if USE_EVDEV
#define LV_USE_BENCHMARK   0
#else
#define LV_USE_BENCHMARK   1
#endif  /*USE_EVDEV*/

/*A demo application with Keyboard, Text area, List and Chart
 * placed on Tab view */
#define LV_USE_DEMO        1
#if LV_USE_DEMO
#define LV_DEMO_WALLPAPER  1    /*Create a wallpaper too*/
#define LV_DEMO_SLIDE_SHOW 0    /*Automatically switch between tabs*/
#endif

/*MCU and memory usage monitoring*/
#if USE_EVDEV
#define LV_USE_SYSMON      0
#else
#define LV_USE_SYSMON      1
#endif  /*USE_EVDEV*/

/*A terminal to display received characters*/
#if USE_EVDEV
#define LV_USE_TERMINAL    0
#else
#define LV_USE_TERMINAL    1
#endif  /*USE_EVDEV*/

/*Touch pad calibration with 4 points*/
#if USE_EVDEV
#define LV_USE_TPCAL       0
#else
#define LV_USE_TPCAL       1
#endif  /*USE_EVDEV*/

#endif /*LV_EX_CONF_H*/

