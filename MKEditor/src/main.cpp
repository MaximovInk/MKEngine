#include "mkengine.h"
#include <iostream>

int main() {

	MK_LOG_INIT();

	MK_INFO("Info {}", 0.6);	//not enabled logging, so not executed

	system("pause");
	return 0;
}