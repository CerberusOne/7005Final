#ifndef EMULATOR_H
#define EMULATOR_H

#include "../Library/socketwrappers.h"
#include "../Library/client.h"
#include "../Library/server.h"
#include "../Library/library.h"
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

using namespace std;

class emulator
{
	emulator();
	~emulator(){
	}
};

#endif
