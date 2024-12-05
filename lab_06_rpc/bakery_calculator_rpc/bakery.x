enum OPERATION
{
    PLUS,
    MINUS,
    MULTIPLICATION,
    DIVISION
};

struct REQUEST
{
    int index;
    int pid;
    float arg1;
    enum OPERATION operation;
    float arg2;
    float res;
    int number;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        int GET_NUMBER(struct REQUEST) = 1;
        struct REQUEST BAKERY_SERVICE(struct REQUEST) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001; /* RPC program number */