/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * example-server.cpp
 *
 *      Author: smeshlink
 */

#include "contiki-arduino.h"
#include "CoAPServer.h"

static void get(CoAPRequest& request, CoAPResponse& response)
{
	response.setContentType(REST.type.TEXT_PLAIN);
	response.print("GET");
}
static void post(CoAPRequest& request, CoAPResponse& response)
{
	response.setContentType(REST.type.TEXT_PLAIN);
	response.print("POST");
}
static void put(CoAPRequest& request, CoAPResponse& response)
{
	response.setContentType(REST.type.TEXT_PLAIN);
	response.print("PUT");
}
static void del(CoAPRequest& request, CoAPResponse& response)
{
	response.setContentType(REST.type.TEXT_PLAIN);
	response.print("DELETE");
}
CoAPResource resTest("test", "title=\"Default test resource\"", get, post, put, del);

static int counter = 0;
static void obs(CoAPRequest& request, CoAPResponse& response)
{
	response.setContentType(REST.type.TEXT_PLAIN);
	response.setMaxAge(1);
	response.printf("TICK %d", counter++);
}
PeriodicResource resObs("obs", CLOCK_SECOND, "title=\"Observable resource which changes every second\";obs;rt=\"observe\"", obs);

void setup()
{
	CoAPServer.init();
	CoAPServer.addResource(&resTest);
	CoAPServer.addResource(&resObs);
}
