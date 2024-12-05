// bakery_server.c

#include "bakery.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_THREADS 1000

int choosing[MAX_THREADS] = {0};
int number_array[MAX_THREADS] = {0};

pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_id = 0;

int get_thread_id() {
    pthread_mutex_lock(&id_mutex);
    if (next_id >= MAX_THREADS) {
        next_id = 0;
    }
    int id = next_id++;
    pthread_mutex_unlock(&id_mutex);
    return id;
}

void bakery_lock(int i) {
    choosing[i] = 1;
    int max = 0;
    for(int j = 0; j < MAX_THREADS; j++)
        if(number_array[j] > max)
            max = number_array[j];

    number_array[i] = max + 1;
    choosing[i] = 0;
}


void bakery_unlock(int i) {
    number_array[i] = 0;
}

int *
get_number_1_svc(struct REQUEST *argp, struct svc_req *rqstp)
{
    static int result;
    int thread_id = get_thread_id();

    bakery_lock(thread_id);

    result = number_array[thread_id];

    printf("Присвоен номер клиента: %d\n", result);
    return &result;
}

struct REQUEST *
bakery_service_1_svc(struct REQUEST *argp, struct svc_req *rqstp)
{
    static struct REQUEST result;
	
	int i = result.number;

	for(int j = 0; j < MAX_THREADS; j++) {
        if(j == i) continue;
        while(choosing[j])
            // Nothing
        while(number_array[j] != 0 && (number_array[j] < number_array[i] || 
              (number_array[j] == number_array[i] && j < i))) {
            // Nothing
		}
    }

    result = *argp;

    switch(argp->operation)
    {
        case PLUS:
            result.res = argp->arg1 + argp->arg2;
            break;
        case MINUS:
            result.res = argp->arg1 - argp->arg2;
            break;
        case MULTIPLICATION:
            result.res = argp->arg1 * argp->arg2;
            break;
        case DIVISION:
            if(argp->arg2 != 0)
                result.res = argp->arg1 / argp->arg2;
            else {
                fprintf(stderr, "Ошибка: Деление на ноль от клиента %d.\n", result.number);
                result.res = 0;
            }
            break;
        default:
            fprintf(stderr, "Неизвестная операция от клиента %d.\n", result.number);
            result.res = 0;
            break;
    }

    printf("Выполнена операция от клиента %d: %.2f %c %.2f = %.2f\n", 
           result.number, result.arg1, result.operation, result.arg2, result.res);

    return &result;
}