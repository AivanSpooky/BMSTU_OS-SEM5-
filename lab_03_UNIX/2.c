#include <stdio.h>
#include <math.h>
#include <unistd.h>

#define OK 0
#define ERR_IO 1
#define ERR_WRONG_INPUT 2

int function(double *sum)
{
	double cur_x = 0;
	int n = 1;
	printf("pid = %d, Введите положительный икс (отрицательный - конец ввода): \n", getpid());
	if (scanf("%lf", &cur_x) != 1)
	{
		return ERR_IO;
	}
	if (cur_x < 0)
	{
		return ERR_WRONG_INPUT;
	}
	else
	{
		*sum += sqrt(cur_x / n);
		n++;
	}
	while (n > 0)
	{
		printf("pid = %d, Введите неотрицательный икс (отрицательный - конец ввода): \n", getpid());
		if (scanf("%lf", &cur_x) != 1)
		{
			return ERR_IO;
		}
		if (cur_x < 0)
		{
			break;
		}
		else
		{
			*sum += sqrt(cur_x / n);
			n++;
		}
	}
	*sum = sin(*sum);
	return OK;
}

int main(void)
{
	double sum = 0;

	int err_code = function(&sum);
	switch (err_code)
	{
		case OK:
			printf("pid = %d, %f\n", getpid(), sum);
			break;
		case ERR_IO:
			printf("ERR_IO\n");
		case ERR_WRONG_INPUT:
			printf("Wrong input error! \n");
	}

	return err_code;
}
