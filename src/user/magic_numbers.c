#include <magic_numbers.h>

#include <my_assert.h>
#include <track_data.h>

int LOOP[] = {57, 95, 96, 52, 69, 98, 51, 21, 105, 43, 107, 3, 31, 108, 36, 90, 46, 59, 93, 74};
int LOOP_LEN = 20;


int get_stopping(char train, char speed){
    if (which_track == 'a') {
        if (train == 0) {
            if (speed == 10) {
                return 435000;
            } else if (speed == 12) {
                return 773000;
            }
        } else if (train == 1) {
            if (speed == 10) {
                return 448000;
            }
        }
    } else if (which_track == 'b') {
        if (train == 0) {
            if (speed == 10) {
                return 405000;
            }
        } else if (train == 1) {
            if (speed == 10) {
                return 448000;
            }
        }
    }
    KASSERT(0, "Can't stop this train");
}
