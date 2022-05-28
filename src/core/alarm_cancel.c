#include "../../include/core/alarm_cancel.h"

/**
 * @brief method to cancel alarm.
 * 
 */
void cancel(int *index, struct alarm *alarms) {
    printf("Cancel which alarm? ");

    int given_alarm_number;
    scanf("%1d", &given_alarm_number);
    given_alarm_number = given_alarm_number - 1;

    //Checks if alarm is sucessfully terminated
    if (kill(alarms[given_alarm_number].pid, SIGTERM) == 0)
    {
        printf("Alarm %d is canceled.\n", given_alarm_number + 1);
        *index -= 1;
    } else {
        printf("Something went wrong");
    }
}