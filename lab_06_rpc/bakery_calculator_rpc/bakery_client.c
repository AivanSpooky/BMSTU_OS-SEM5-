// bakery_client.c

#include "bakery.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char get_operation_char(enum OPERATION op) {
    switch(op) {
        case PLUS:
            return '+';
        case MINUS:
            return '-';
        case MULTIPLICATION:
            return '*';
        case DIVISION:
            return '/';
        default:
            return '?';
    }
}

int main (int argc, char *argv[])
{
    CLIENT *clnt;
    char *host;
    int *number;
    struct REQUEST req;
    struct REQUEST *resp;

    srand(time(NULL));

    if (argc < 2) {
        fprintf (stderr, "Использование: %s сервер_host\n", argv[0]);
        exit (1);
    }
    host = argv[1];

    clnt = clnt_create (host, BAKERY_PROG, BAKERY_VER, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror (host);
        exit (1);
    }

    while(1) {
        memset(&req, 0, sizeof(req));
        number = get_number_1(&req, clnt);
        if (number == (int *) NULL) {
            clnt_perror (clnt, "Ошибка при вызове GET_NUMBER");
            sleep(1); 
            continue;
        }
        req.number = *number;
        printf("Ваш номер клиента: %d\n", req.number);

        req.pid = getpid();

        req.arg1 = ((float)(rand() % 1000)) / 10.0; 
        req.arg2 = ((float)(rand() % 1000)) / 10.0; 
        req.operation = (enum OPERATION)(rand() % 4); 

        if(req.operation == DIVISION && req.arg2 == 0.0) {
            req.arg2 = ((float)(rand() % 1000)) / 10.0;
            if(req.arg2 == 0.0) req.arg2 = 1.0; 
        }

        resp = bakery_service_1(&req, clnt);
        if (resp == (struct REQUEST *) NULL) {
            clnt_perror (clnt, "Ошибка при вызове BAKERY_SERVICE");
            sleep(1);
            continue;
        }

        char op_char = get_operation_char(resp->operation);

        printf("%.2f %c %.2f = %.2f\n", resp->arg1, op_char, resp->arg2, resp->res);

        sleep(rand() % 3 + 1);
    }

    clnt_destroy(clnt);
    return 0;
}