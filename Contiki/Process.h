/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * Process.h
 *
 *      Author: smeshlink
 */

#ifndef PROCESS_H_
#define PROCESS_H_

extern "C" {
#include "contiki.h"
}

class Process {
private:
	static Process *list;
	struct process _process;
	Process *_next;
	PROCESS_THREAD(process_manager, ev, data);
	static void listAdd(Process *proc);
	static Process* listFind(struct process *p);
public:
	Process(const char *name = NULL);
	virtual ~Process();
	void run(const char *arg = NULL);
protected:
	virtual PT_THREAD(thread(struct pt *process_pt, process_event_t ev, process_data_t data)) = 0;
};

#endif /* PROCESS_H_ */
