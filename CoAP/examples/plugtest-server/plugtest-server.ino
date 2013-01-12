/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * plugtest-server.cpp
 * 
 *      Author: smeshlink
 */

#include <string.h>

#include "CoAPServer.h"
#include "contiki-arduino.h" //it is important, do not remove

/* Define which resources to include to meet memory constraints. */
#define REST_RES_TEST 1
#define REST_RES_LONG 1
#define REST_RES_QUERY 1
#define REST_RES_SEPARATE 0
#define REST_RES_LARGE 1
#define REST_RES_LARGE_UPDATE 0
#define REST_RES_LARGE_CREATE 1
#define REST_RES_OBS 1

#if REST_RES_TEST
/*
 * Default test resource
 */
class TestResource : public CoAPResource
{
public:
	TestResource() : CoAPResource("test")
	{
		setAttributes("title=\"Default test resource\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("Type: %u\nCode: %u\nMID: %u", request.getType(), request.getCode(), request.getId());
	}

	virtual void doPost(CoAPRequest& request, CoAPResponse& response)
	{
		response.setStatusCode(REST.status.CREATED);
		response.setLocation("/nirvana");
	}

	virtual void doPut(CoAPRequest& request, CoAPResponse& response)
	{
		response.setStatusCode(REST.status.CHANGED);
	}

	virtual void doDelete(CoAPRequest& request, CoAPResponse& response)
	{
		response.setStatusCode(REST.status.DELETED);
	}
} resTest;
#endif

#if REST_RES_LONG
/*
 * Long path resource
 */
class LongPathResource : public CoAPResource
{
public:
	LongPathResource() : CoAPResource("seg1/seg2/seg3")
	{
		setAttributes("title=\"Long path resource\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("Type: %u\nCode: %u\nMID: %u", request.getType(), request.getCode(), request.getId());
	}
} resLongPath;
#endif

#if REST_RES_QUERY
/*
 * Resource accepting query parameters
 */
class QueryResource : public CoAPResource
{
public:
	QueryResource() : CoAPResource("query")
	{
		setAttributes("title=\"Resource accepting query parameters\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		int len = 0;
		const char *query = NULL;
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("Type: %u\nCode: %u\nMID: %u", request.getType(), request.getCode(), request.getId());
		if ((len = request.getQuery(&query)))
			response.printf("\nQuery: %s", len, query);
	}
} resQuery;
#endif

#if REST_RES_SEPARATE
#warning "Separate resource has not been implemented yet"
#endif

#if REST_RES_LARGE
/*
 * Large resource
 */
#define CHUNKS_TOTAL 1280

class LargeResource : public CoAPResource
{
public:
	LargeResource() : CoAPResource("large")
	{
		setAttributes("title=\"Large resource\";rt=\"block\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		int32_t strpos = 0;
		int32_t offset = response.getOffset();

		if (offset >= CHUNKS_TOTAL)
		{
			response.setStatusCode(REST.status.BAD_OPTION);
			/* A block error message should not exceed the minimum block size (16). */
			const char *error_msg = "BlockOutOfScope";
			response.print(error_msg);
			return;
		}

		/* Generate data until reaching CHUNKS_TOTAL. */
		while (strpos < request.preferredSize)
		{
			strpos += response.printf("|%ld|", offset);
		}

		/* snprintf() does not adjust return value if truncated by size. */
		if (strpos > request.preferredSize)
		{
			strpos = request.preferredSize;
		}

		/* Truncate if above CHUNKS_TOTAL bytes. */
		if (offset + (int32_t)strpos > CHUNKS_TOTAL)
		{
			strpos = CHUNKS_TOTAL - offset;
		}

		response.setPayloadLength(strpos);
		response.setContentType(REST.type.TEXT_PLAIN);

		/* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
		response.blockAppend(strpos);

		/* Signal end of resource representation. */
		if (response.getOffset() >= CHUNKS_TOTAL)
		{
			response.blockComplete();
		}
	}
} resLarge;
#endif

#if REST_RES_LARGE_UPDATE
/*
 * Large resource that can be updated using PUT method
 */
#define STORE_SIZE 1280
class LargeUpdateResource : public CoAPResource
{
private:
	static int32_t largeUpdateSize;
	static uint8_t largeUpdateStore[STORE_SIZE];
	static unsigned int largeUpdateCt;
public:
	LargeUpdateResource() : CoAPResource("largeUpdate")
	{
		setAttributes("title=\"Large resource that can be updated using PUT method\";rt=\"block\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		if (response.getOffset() >= largeUpdateSize)
		{
			response.setStatusCode(REST.status.BAD_OPTION);
			/* A block error message should not exceed the minimum block size (16). */
			const char *error_msg = "BlockOutOfScope";
			response.print(error_msg);
			return;
		}

		response.write((char *)largeUpdateStore + response.getOffset(), request.preferredSize);
		response.setContentType(largeUpdateCt);

		/* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
		response.blockAppend(request.preferredSize);

		/* Signal end of resource representation. */
		if (response.getOffset() >= largeUpdateSize)
		{
			response.blockComplete();
		}
	}

	virtual void doPut(CoAPRequest& request, CoAPResponse& response)
	{
		uint8_t *incoming = NULL;
		size_t len = 0;

		unsigned int ct = request.getContentType();
		if (ct == (unsigned int)-1)
		{
			response.setStatusCode(REST.status.BAD_REQUEST);
			const char *error_msg = "NoContentType";
			response.print(error_msg);
			return;
		}

		if ((len = request.getPayload(&incoming)))
		{
		  if (request.getBlock1Num() * request.getBlock1Size() + len <= sizeof(largeUpdateStore))
		  {
			memcpy(largeUpdateStore + request.getBlock1Num() * request.getBlock1Size(), incoming, len);
			largeUpdateSize = request.getBlock1Num() * request.getBlock1Size() + len;
			largeUpdateCt = request.getContentType();

			response.setStatusCode(REST.status.CHANGED);
			response.setBlock1(request.getBlock1Num(), 0, request.getBlock1Size());
		  }
		  else
		  {
			response.setStatusCode(REST.status.REQUEST_ENTITY_TOO_LARGE);
			response.printf("%uB max.", sizeof(largeUpdateStore));
			return;
		  }
		}
		else
		{
			response.setStatusCode(REST.status.BAD_REQUEST);
			const char *error_msg = "NoPayload";
			response.print(error_msg);
			return;
		}
	}
} resLargeUpdate;
int32_t LargeUpdateResource::largeUpdateSize = 1280;
uint8_t LargeUpdateResource::largeUpdateStore[STORE_SIZE] = {0};
unsigned int LargeUpdateResource::largeUpdateCt = REST.type.APPLICATION_OCTET_STREAM;
#endif

#if REST_RES_LARGE_CREATE
/*
 * Large resource that can be created using POST method
 */
class largeCreateResource : public CoAPResource
{
public:
	largeCreateResource() : CoAPResource("large-create")
	{
		setAttributes("title=\"Large resource that can be created using POST method\";rt=\"block\"");
	}
protected:
	virtual void doPost(CoAPRequest& request, CoAPResponse& response)
	{
		uint8_t *incoming = NULL;
		size_t len = 0;

		unsigned int ct = request.getContentType();
		if (ct == (unsigned int)-1)
		{
			response.setStatusCode(REST.status.BAD_REQUEST);
			const char *error_msg = "NoContentType";
			response.print(error_msg);
			return;
		}

		if ((len = request.getPayload(&incoming)))
		{
		  if (request.getBlock1Num() * request.getBlock1Size() + len <= 1024)
		  {
			response.setStatusCode(REST.status.CREATED);
			response.setLocation("/nirvana");
			response.setBlock1(request.getBlock1Num(), 0, request.getBlock1Size());
		  }
		  else
		  {
			response.setStatusCode(REST.status.REQUEST_ENTITY_TOO_LARGE);
			response.printf("%uB max.", 1024);
			return;
		  }
		}
		else
		{
			response.setStatusCode(REST.status.BAD_REQUEST);
			const char *error_msg = "NoPayload";
			response.print(error_msg);
			return;
		}
	}
} resLargeCreate;
#endif

#if REST_RES_OBS
/*
 * Observable resource which changes every 3 seconds
 */
class ObserveResource : public PeriodicResource
{
public:
	ObserveResource() : PeriodicResource("obs", 3 * CLOCK_SECOND)
	{
		setAttributes("title=\"Observable resource which changes every 3 seconds\";obs;rt=\"observe\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		response.setContentType(REST.type.TEXT_PLAIN);
		response.setMaxAge(5);
		response.printf("TICK %d", obsCounter);
	}
#if 0
	/*
	 * Perform GET by default, override this method to behavior different,
	 * and return false if DO NOT want to trigger this time.
	 * */
	virtual bool periodic(CoAPResponse& response)
	{
		if (obsCounter % 2)
		{
			response.setContentType(REST.type.TEXT_PLAIN);
			response.setMaxAge(3);
			response.printf("TICK period %d", obsCounter);
			return true;
		}
		else
			return false;
	}
#endif
} resObservable;
#endif

void config()
{
}

void setup()
{
	CoAPServer.init();
#if REST_RES_TEST
	CoAPServer.addResource(&resTest);
#endif
#if REST_RES_LONG
	CoAPServer.addResource(&resLongPath);
#endif
#if REST_RES_QUERY
	CoAPServer.addResource(&resQuery);
#endif
#if REST_RES_LARGE
	CoAPServer.addResource(&resLarge);
#endif
#if REST_RES_LARGE_UPDATE
	CoAPServer.addResource(&resLargeUpdate);
#endif
#if REST_RES_LARGE_CREATE
	CoAPServer.addResource(&resLargeCreate);
#endif
#if REST_RES_OBS
	CoAPServer.addResource(&resObservable);
#endif
}
