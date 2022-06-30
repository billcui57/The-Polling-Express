#include <magic_numbers.h>

#include <my_assert.h>


int LOOP[] = {72, 95, 96, 52, 69, 98, 51, 21, 105, 43, 107, 3, 31, 108, 41, 110, 16, 61, 113, 77};
int LOOP_LEN = 20;
int LOOP_DIST = 4663000;


int get_stopping(char track, char train, char speed){
    if (track == 0) {
        if (train == 1) {
            if (speed == 10) {
                return 435000;
            } else if (speed == 12) {
                return 773000;
            }
        } else if (train == 24) {
            if (speed == 10) {
                return 220000;
            }
        }
    } else if (track == 1) {
        if (train == 1) {
            if (speed == 10) {
                return 435000;
            }
        }
    }
    KASSERT(0, "Can't stop this train");
}
