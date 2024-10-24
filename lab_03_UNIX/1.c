#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define OK 0
#define ERR_IO 1
#define ERR_RANGE 2
#define ERR_EMPTY 3

#define N 10
// Функция ввода массива
int input(int a[], size_t *a_size)
{
	printf("pid = %d, Input n: ", getpid());
	if (scanf("%zu", a_size) != 1)
		return ERR_IO;
	if (*a_size == 0 || *a_size > N)
		return ERR_RANGE;
	printf("pid = %d, Input %zu elements: ", getpid(), *a_size);
	for (size_t i = 0; i < *a_size; i++)
		if (scanf("%d", &a[i]) != 1)
			return ERR_IO;
	return OK;
}
// Функция вывода массива
void print(const int a[], size_t a_size)
{
	printf("pid = %d, \nArray: \n", getpid());
	for (size_t i = 0; i < a_size; i++)
		printf("%d ", a[i]);
	printf("\n");
}
// Проверка числа на простоту
bool is_prime(int num)
{
	if (num <= 1)
		return false;
	for (int i = 2; i <= sqrt(num); i++)
		if (num % i == 0)
			return false;
	return true; 
}
// Добавление простых чисел старого массива в новый
void add_primes_from_old_to_new(const int a[], size_t a_size, int b[], size_t *b_size)
{
	for (size_t i = 0; i < a_size; i++)
		if (is_prime(a[i]))
		{
			b[*b_size] = a[i];
			(*b_size)++;
		}
}

int main(void)
{
	int a[N];
	size_t a_size = 0;
	int b[N];
	size_t b_size = 0;
	
	int error = input(a, &a_size);
	if (error != 0)
	{
		switch (error)
		{
			case ERR_IO:
				printf("ERR_IO\n");
				break;
			case ERR_RANGE:
				printf("ERR_RANGE\n");
				break;
		}
		return error;
	}
	add_primes_from_old_to_new(a, a_size, b, &b_size);
	if (b_size == 0)
	{	
		printf("ERR_EMPTY\n");
		return ERR_EMPTY;
	}
	print(b, b_size);
	
	return OK;
}
