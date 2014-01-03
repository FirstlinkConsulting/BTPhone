#include "Fwl_Keypad.h"

#define KEYPAD_TIMER_DELAY          20
#define DEFAULT_LONG_KEY_DELAY      1000 //in 1s
#define DEFAULT_INTERVAL_KEY_DELAY  120  //120ms
#define DEFAULT_PRESSING_KEY_DELAY  200
#define DEFAULT_TREMBLE_KEY_DELAY   20   //in 20ms

// *** keypad
typedef struct {
    T_U16       left;
    T_U16       top;
    T_U16       right;
    T_U16       bottom;
    T_eKEY_ID   key_id;
} T_WIN_KEY;

T_VOID w_keypad_init();
T_eKEY_ID w_keypad_scan(T_VOID);
