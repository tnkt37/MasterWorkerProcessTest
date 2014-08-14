#include <iostream>
#include "Python.h"
using namespace std;
int main()
{
	Py_Initialize();
	PyRun_SimpleString("from time import time,ctime\n"
		"print 'Today is',ctime(time())\n");
	Py_Finalize();
	cin >> __argc;
	return 0;
}