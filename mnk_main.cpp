// IKPI93_Koltunova_EV_AOPI_LR_1.c : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <windows.h>

using namespace std;

class func_context {

public:

	func_context(int M, int N, ifstream& file_data) : M(M), N(N) {
		allocate_memory();
		read_points(file_data);
	}

	~func_context() {
		free_memory();
	}

	//выделение памяти
	int allocate_memory() {
		a = new float[M + 1];
		b = new float[M + 1];
		x = new float[N];
		y = new float[N];
		sum = new float*[M + 1];
		if (NULL == x || NULL == y || NULL == a || NULL == sum) {
			printf("\nНедостаточно памяти. \nN = %d, M = %d\n", N, M);
			return -1;
		}

		for (int n = 0; n < (M + 1); n++) {
			sum[n] = new float[M + 1];
			if (NULL == sum[n]) {
				printf("\nНедостаточно памяти.\n", M + 1);
			}
		}
		for (int n = 0; n < M + 1; n++) {
			a[n] = 0;
			b[n] = 0;
			for (int m = 0; m < M + 1; m++) {
				sum[n][m] = 0;
			}
		}
		for (int n = 0; n < N; n++) {
			x[n] = 0;
			y[n] = 0;
		}
	}

	//освобождение памяти
	void free_memory() {
		for (int n = 0; n < (M + 1); n++) {
			delete[] sum[n];
		}
		delete[] a;
		delete[] b;
		delete[] x;
		delete[] y;
		delete[] sum;
	}


	//чтение точек из файла
	void read_points(ifstream& file_data) {
		//read points
		file_data.clear();
		file_data.seekg(0, file_data.beg);

		for (int n = 0; n < N; n++) {

			try
			{
				file_data >> sx >> sy;

				//fscanf(file_data, "%c", &sx[n]);
				//	fscanf(file_data, "%c", &sy[n]);
				// Блок проверки на наличие нуля
				if (sx[0] == '0')
				{
					x[n] = 0;
					zero = false;
				}
				if (sy[0] == '0')
				{
					y[n] = 0;
					zero = false;
				}

				// Блок проверки корректности ввода данных
				if ((!atof(sx) || !atof(sy)) && zero)
				{
					throw 3; // передать код ошибки 3
				}
				else
				{
					x[n] = atof(sx);
					y[n] = atof(sy);
				}

				// Проверка на выход за пределы максимального значения
				if (x[n] > (pow(2, sizeof(float) * 8.0 - 1) - 1) || y[n] > (pow(2, sizeof(float) * 8.0 - 1) - 1))
				{
					throw 5;
				}

				// Проверка на выход за пределы минимального значения
				if (x[n] < (-1 * (pow(2, sizeof(float) * 8.0 - 1))) || y[n] < (-1 * (pow(2, sizeof(float) * 8.0 - 1))))
				{
					throw 7;
				}
			}
			catch (int i) // Получение кода ошибки
			{
				if (i == 3)
				{
					cout << "Ошибка: Введена буква" << endl;
				}
				else if (i == 5)
				{
					cout << "Ошибка: Выход за пределы диапазона" << endl;
				}
				else if (i == 7)
				{
					cout << "Ошибка: Выход за пределы диапазона" << endl;
				}
				continue;
			}
		}


	}


	float* a;
	float* b;
	float* x;
	float* y;
	float** sum;
	char sx[64], sy[64];
	bool zero = true;
	int N, M;
};

char file_name[256];

CRITICAL_SECTION csFlag;

HANDLE hEvent;

//подсчет точек в файле
void amount_points(ifstream& file_data, int& N) {

	string s;

	N = 0;

	while (file_data.peek() != EOF) {
		getline(file_data, s);
		N++;
	}

	cout << "\nКоличество точек: N = " << N << endl;
}


//составление системы уравнений для полинома
void system_polinom(func_context& context) {

	for (int n = 0; n < (context.M + 1); n++) {
		for (int m = 0; m < (context.M + 1); m++) {
			context.sum[n][m] = 0;
			for (int j = 0; j < context.N; j++) {
				context.sum[n][m] += pow(context.x[j], n + m);
			}
		}
	}
	//free coefficients(b)
	for (int n = 0; n < (context.M + 1); n++) {
		for (int m = 0; m < context.N; m++) {
			context.b[n] += pow(context.x[m], n) * context.y[m];
		}
	}
}

//избавление от нулей на главной диагонали
void without_zeros(func_context& context) {
	float temp = 0;

	for (int n = 0; n < (context.M + 1); n++) {
		if (context.sum[n][n] == 0)
		{
			for (int m = 0; m < (context.M + 1); m++) {
				if (n == m) {
					continue;
				}
				if (context.sum[n][m] != 0 && context.sum[m][n] != 0) {
					for (int j = 0; j < (context.M + 1); j++) {
						temp = context.sum[m][j];
						context.sum[m][j] = context.sum[n][j];
						context.sum[n][j] = temp;
					}
					temp = context.b[m];
					context.b[m] = context.b[n];
					context.b[n] = temp;
					break;
				}
			}
		}
	}
}

//ввод степени
int input_power(int N) {
	printf("\nВведите степень аппроксимирующей функции: M = ");
	int M = 0;
	scanf("%d", &M);
	while (M >= N) {
		printf("\nОшибка! Степень функции должна быть меньше количества точек N = %d.", N);
		printf("\nПопробуйте еще раз.\nВведите степень: M = ");			
		scanf("%d", &M);
	}
	return M;
}

//вычисление коэффициентов
void array_coef(func_context& context) {
	for (int n = context.M; n >= 0; n--) {
		float s = 0;
		for (int m = n; m < (context.M + 1); m++) {
			s = s + context.sum[n][m] * context.a[m];
		}
		context.a[n] = (context.b[n] - s) / context.sum[n][n];
	}
}

//вывод коэффициентов
void output_result_coef(func_context& context) {
	printf("\nКоэффициенты аппроксимирующей функции:");
	for (int n = 0; n < (context.M + 1); n++) {
		printf("\na[%d] = %.3f", n, context.a[n]);
	}
	printf("\n");
}

//вывод вида полинома
void output_polinom(func_context& context) {

	printf("\nАппроксимирующий полином %iй степени:", context.M);
	printf("\nf(x) = ");
	for (int n = 0; n <= context.M; n++) {
		printf("a[%i]*x^%i", n, n);
		if (n < context.M) {
			printf(" + ");
		}
	}
	printf("\n");
}

//вывод итогового полинома
void output_result_polinom(func_context& context){
	printf("\nРезультат:");
	printf("\nf(x) = ");
	for (int n = 0; n <= context.M; n++) {
		printf("%.3f*x^%i", context.a[n], n);
		if (n < context.M) {
				printf(" + ");
		}
	}
	printf("\n");
}

//решение системы методом Гаусса
void process(func_context& context) {
	
	for (int n = 0; n < context.M + 1; n++) {
		for (int m = (n + 1); m < (context.M + 1); m++) {
			if (context.sum[m][m] == 0) {
				printf("\nРешение не существует.\n");
				return;
			}

			float K = context.sum[m][n] / context.sum[n][n];
			for (int j = n; j < (context.M + 1); j++) {
				context.sum[m][j] -= K * context.sum[n][j];
			}
			context.b[m] -= K * context.b[n];
		}
	}
}



//вывод аппроксимирующей функции (x^M)*sin(M*x)
void output_result_sin(func_context& context) {
	printf("\nРезультат:");
	printf("\nf(x) = ");
	for (int n = 0; n < (context.M+1); n++) {
		printf("%.3f*(x^%i) * sin(%i * x)", context.a[n], context.M+1, context.M+1);
		if (n < context.M) {
				printf(" + ");
		}
	}
	printf("\n\n");
}

//составление системы уравнений для функции (x^M)*sin(M*x)
void system_sin(func_context& context) {
	for (int n = 0; n <= context.M; n++) {
		for (int m = 0; m <= context.M; m++) {
			context.sum[n][m] = 0;
			for (int j = 0; j < context.N; j++) {
				context.sum[n][m] += pow(context.x[j], n+1) * sin((n+1) * context.x[j]) * pow(context.x[j], m+1) * sin((m+1)* context.x[j]);
			}
		}
	}

	//free coefficients(b)
	for (int n = 0; n <= context.M; n++) {
		context.b[n] = 0;
		for (int j = 0; j < context.N; j++) {
			context.b[n] += (pow(context.x[j], (n+1))* sin((n+1) * context.x[j]) * context.y[j]);
		}
	}
}

DWORD WINAPI Thread_power(LPVOID lpParam) {

	func_context& context = *reinterpret_cast<func_context*>(lpParam);

	system_polinom(context);
	without_zeros(context);
	process(context);
	array_coef(context);

	EnterCriticalSection(&csFlag);

	cout << endl << "Аппроксимация МНК с помощью степенной функции" << endl;
	output_result_coef(context);
	output_result_polinom(context);
	LeaveCriticalSection(&csFlag);


	SetEvent(hEvent);

	return 0;
}

DWORD WINAPI Thread_sin(LPVOID lpParam) {
	
	WaitForSingleObject(hEvent, INFINITE);

	func_context& context = *reinterpret_cast<func_context*>(lpParam);

	system_sin(context);
	without_zeros(context);

	process(context);
	array_coef(context);

	EnterCriticalSection(&csFlag);

	cout << endl << "Аппроксимация МНК с помощью функции (x^M)*sin(M*x)" << endl;
	output_result_coef(context);
	output_result_sin(context);
	LeaveCriticalSection(&csFlag);

	return 0;
}


void print_times(HANDLE hThread) {
	FILETIME  ftimeCreate, ftimeExit, ftimeKernel, ftimeUser;
	BOOL ok = GetThreadTimes(hThread, &ftimeCreate, &ftimeExit, &ftimeKernel, &ftimeUser);
	cout << (((uint64_t)(ftimeExit.dwHighDateTime - ftimeCreate.dwHighDateTime) << MAXDWORD) +
		ftimeExit.dwLowDateTime - ftimeCreate.dwLowDateTime) / 10 << " uSec "
		<< endl;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	cout << "Введите название файла: ";
	cin >> file_name;

	ifstream file_data(file_name);

	while (!(file_data.is_open())) {
		cout << "Введите название файла: ";
		cin >> file_name;
		ifstream file_data(file_name);
	}

	int N = 0;
	amount_points(file_data, N);
	

	int M = input_power(N);

	func_context context_sin(M - 1, N, file_data);
	func_context context_power(M, N, file_data);

	file_data.close();

	HANDLE fThread, sThread;
	DWORD ThreadID, dwRet;
	InitializeCriticalSection(&csFlag); //инициализациия критической секции

	hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	fThread = CreateThread(
		NULL,
		0,
		Thread_power,
		&context_power,
		CREATE_SUSPENDED,
		&ThreadID);

	if (fThread == NULL) {
		cout << "Thread Power Creation Failed & Error No ---> " << GetLastError() << endl;
	}

	cout << "\nThread Power Creation Success" << endl;
	cout << "Thread Power ID --> " << ThreadID << endl;

	sThread = CreateThread(
		NULL,
		0,
		Thread_sin,
		&context_sin,
		CREATE_SUSPENDED,
		&ThreadID);

	if (sThread == NULL) {
		cout << "Thread Sin Creation Failed & Error No ---> " << GetLastError() << endl;
	}
	cout << "\nThread Sin Creation Success" << endl;
	cout << "Thread Sin ID --> " << ThreadID << endl;


	ResumeThread(fThread);
	ResumeThread(sThread);

	dwRet = WaitForSingleObject(fThread, INFINITE);
	dwRet = WaitForSingleObject(sThread, INFINITE);

	cout << "P time:"; print_times(fThread);
	cout << "S time:"; print_times(sThread);

	CloseHandle(fThread);
	CloseHandle(sThread);
	CloseHandle(hEvent);
	
	return 0;
}

