struct {
int8_t task_number;
int16_t delay;
int16_t interval;
int8_t run;
} Task_list[TASK_MAX];

#define TASK_1 1
#define TASK_2 2
#define TASK_3 3

void add_task(int8_t number, int8_t task_number, int16_t delay, int 16_t interval)
{
    Task_list[number].task_number = task_number;
    Task_list[number].delay= delay;
    Task_list[number].interval=interval;
}

int update()
{
    for(i=0;i<TASK_MAX;i++)
    { // check every task
        if(Task_list[i].task_number != 0)
        { // not an empty slot
            if(Task_list[i].delay == 0)
            { // time to run
                Task_list[i].run += 1; // please run this
                if(Task_list[i].interval != 0) // periodic task
                    Task_list[i].delay = Task_list[i].interval;
            }
                else
                    Task_list[i].delay -=1; // decrement waiting time
        }
    }
}

int dispatcher()
{
    for(i=0;i<TASK_MAX;i++)
    { // check every task
        if(Task_list[i].task_number != 0)
        { // not an empty slot
            if(Task_list[i].run > 0)
            { // time to run
                // Call the task function. This can be done more elegantly
                // using function pointers
                if(Task_list[i].task_number == TASK_1) Task_1();
                else if(Task_list[i].task_number == TASK_2) Task_2();
                else if(Task_list[i].task_number == TASK_3) Task_3();
                Task_list[i].run -= 1; // decrease run request
            }
        }
    }
}

int main()
{
    Init_task_list(); // no tasks in the list
    update(); // configure timer and interrupts
    Add_task(0, TASK_1, 100, 1000); // add a couple of tasks
    Add_task(1, TASK_2, 10, 100);
    While(1) dispatcher(); // schedule run time
}

ISR (TIMER0_OVF_vect)
{
cli(); // no interrupts while updating the list
update();
sei();
}