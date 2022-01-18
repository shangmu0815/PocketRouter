#include <string.h>

float string_to_float_by_index(char* data, int startIndex, int endIndex) {
    float value = 0.0;
    int i_bit = 1;
    float f_bit = 10.0;
    int i;
    for (i = startIndex; i < endIndex; i++) {
        int c = data[i] - '0';
        if (data[i] == ' ') {
            continue ;
        }
        if (data[i] == '.') {
            i_bit = 0;
            continue ;
        }

        if (c > 9 || c < 0) {
            break ;
        }

        if (i_bit != 0) {
            value = (value * i_bit) + c;
            if (i_bit == 1) {
                i_bit = i_bit * 10;
            }
        } else {
            value = value + (c/f_bit);
            f_bit = f_bit * 10;
        }
    }
    return value;
}

float string_to_float_by_string(char* data, char* startTarget, char* endTartget) {
    char* foundStart = strstr(data, startTarget);
    if (foundStart) {
        char* foundEnd = strstr(foundStart, endTartget);
        if (foundEnd) {
            int endIndex = foundEnd - foundStart;
            return string_to_float_by_index(foundStart, strlen(startTarget), endIndex);
        }
    }
    return 0.0;
}

int string_to_int_by_index(char* data, int startIndex, int endIndex) {
    return (int)string_to_float_by_index(data, startIndex, endIndex);
}

int string_to_int_by_string(char* data, char* startTarget, char* endTartget) {
    return (int)string_to_float_by_string(data, startTarget, endTartget);
}
