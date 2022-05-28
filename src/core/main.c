#include "../../include/core/main.h"

/**
 * @brief structure for alarm.
 * We have the process id and the time stamp.
 */
struct alarm 
{
    pid_t pid;
    time_t alarm;
};

/**
 * @brief Array of the structure alarm. Allows maximum 10 entries/alarms.
 */
struct alarm alarm_arr[10];

/**
 * @brief Method for checking if list is empty. We iterate thorugh the list 
 * and return a number. If sum is 0, the array does not have any elements.
 * 
 * @return sum. 
 */
int isEmpty(){
    int sum = 0;
    for (int i=0; i<10; i++){
        sum += alarm_arr[i].alarm;
    }
    return sum;
}

/**
 * @brief Method for checking if the list is full. If the statement is true, is will set the value to 0 and
 * break the loop. We do not need to check any more as it is a space in the list.
 * 
 * @return value.
 */
int isFull(){
    int value = 0;
    for (int i=0; i<10; i++){
        if (alarm_arr[i].alarm == 0){
            value = 0;
            break;
        } else {
            value = 1;
        }
    }
    return value;
}

/**
 * @brief This is the method that will be called if user choses to list all alarms.
 * The method iterates thorugh the list and displays an alarm like this:
 * "Alarm X at YYYY-MM-DD hh:mm:ss". 
 * 
 * `strftime` is used to convert the timestamp into a readable string.
 * If the list is empty the user will be returned to the menu. The alarm number will be 
 * index + 1, since array indices starts at 0 in computer science. 
 */
void list(){
    if (isEmpty() == 0) {
        printf("The list is empty.\n");
    } else {
        for (int i=0; i<10; i++){
            if (alarm_arr[i].alarm != 0){
                char buff[20];
                time_t t = alarm_arr[i].alarm;
                strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
                int index = i + 1;
                printf("Alarm %d at %s.\n", index, buff);
            }
        }
    }
}

/**
 * @brief This is the method that will be called if user choses to cancel an alarm.
 * The user will be prompted an input where the program will tell the user to 
 * write the alarm number of the alarm to cancel. We use fgets to get the number.
 * 
 * The input, will be subtracted by 1. By using the index we set the specific element to zero. 
 * After this we kill the child process of the specific element using the kill(2) method. 
 * If the process is completed successfully we print out "Alarm X is canceled.".
 */
void cancel(){
    if (isEmpty() == 0) {
        printf("The list is empty.\n");
    } else {
        char buf[2];
        printf("Cancel which alarm? ");
        fgets(buf, 4, stdin);
        int indexOfAlarm;
        sscanf(buf, "%d", &indexOfAlarm);
        indexOfAlarm --;
        alarm_arr[indexOfAlarm].alarm = 0;
        int result = kill(alarm_arr[indexOfAlarm].pid, SIGTERM);
        if (result == 0) {
            printf("Alarm %d is canceled.\n", indexOfAlarm+1);
        }
    }
}

/**
 * @brief This is the method that will be called if user choses to exit out of the program.
 * The program will be exited.
 */
void optOut(){
    printf("Goodbye!\n");
}

/**
 * @brief This is the method that will be called if user choses to schedule an alarm. First of all
 * we use the method called isFull() to check if it any space for a new alarm in the list. If it is full the process will
 * be stopped and the user will be returned to main menu. 
 * 
 * The method calls an input, we have decided to use fgets to get input from the user. If the time that is received
 * is before the current time, we will print out `The alarm must be in the future.` and the user will be returned
 * to the menu.
 * 
 * We use strptime and mktime to format the input into an unix timestamp. We find the difference using difftime(), and 
 * print this out. 
 * 
 * After this we create a child process using fork(). We iterate through the array and at the first empty slot 
 * we add the information. We check if the child process is successfully started and then return to the main process. 
 * The child process will then sleep() for amount of seconds from right now until the alarm should be sounded.
 * After the amount of sleep it is played a sound `rickroll.mp3`. We also have a check to determine what operating system 
 * the user is on, and based on this use the correct player.
 * 
 */
void schedule(){  
    if (isFull() == 1) {
        printf("The list is full.\n");
    } else {
        char buf[20];
        printf("Schedule alarm at which date and time? ");
        fgets(buf, 22, stdin);
        struct tm tm;
        strptime(buf, "%Y-%m-%d %H:%M:%S", &tm);
        time_t t = mktime(&tm);
        time_t today = time(0);
         if (t < today) {
            printf("The alarm must be in the future, and in this format yyyy-mm-dd hh:mm:ss.\n");
            return;
        }
        int diff = difftime(t, today);
        printf("Scheduling alarm in %d seconds.\n", diff);
        pid_t pid = fork();

        for (int i=0; i<10; i++){
            if (alarm_arr[i].alarm == 0){
                alarm_arr[i].alarm = t;
                alarm_arr[i].pid = pid;
                break;
            }
        }
        if (pid != 0) {
            return;
        }
        sleep(diff);
        char *path_to_sound = "src/audio/rickroll.mp3";

        #if __APPLE__
            char *path_to_executable = "/usr/bin/afplay";
            execl(path_to_executable, path_to_executable, path_to_sound, (char *)NULL);
        #elif __linux__
            char *path_to_executable = "/usr/bin/mpg123";
            execl(path_to_executable, path_to_executable, path_to_sound, (char *)NULL);
        #endif
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief Method to kill zombie process.
 * 
 */
void cleanZombieProcess() {
    for (int i = 0; i < sizeof(alarm_arr)/sizeof(alarm_arr[0]); i++) {
        struct alarm *alarm = &alarm_arr[i];

        int status = -1;      
        waitpid(alarm->pid, &status, 1);

        if (WIFEXITED(status)) {
            wait(&status);
        }
    }
}

/**
 * @brief This is the method for opening the menu. This menu will be called infinitely, until the method returns
 * 0. We use fgets to get the input from the user. The input will then be checked by using a switch. Based on what 
 * character is received the correct method willbe called. 
 * 
 * @return 1 or 0 based on if the loop should be broke or not. 
 */
int openMenu() {
    char selected[1];
    printf("Please enter \"s\" (schedule), \"l\" (list), \"c\" (cancel), \"x\" (exit) \n");
    fgets(selected, 3, stdin);
    cleanZombieProcess();
    switch (selected[0])
    {
        case 's':
            schedule();
            return 1;
            break;
        case 'l':
            list();
            return 1;
            break;
        case 'c':
            cancel();
            return 1;
            break;
        case 'x':
            optOut();
            return 0;
            break;
        default:
            printf("Nothing selected! \n");
            return 1;
            break;
    }
}

/**
 * @brief This is the main method, and will display a welcome text. We use strftime to show 
 * the date and time of today. We use a while with 1 to make it looping infinitely. If the called
 * method openMenu() returns 0, the loop will be discontinued. 
 * 
 */
int main(){
    char buff[20];
    time_t now = time(NULL);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("Welcome to the alarm clock! It is currently %s\n", buff);
    while(1) {
        if (openMenu() != 1){
            break;
        }
    }
    return 0;
}