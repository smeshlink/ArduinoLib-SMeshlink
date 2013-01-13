/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * Process.cpp
 *
 *      Author: smeshlink
 */

#include "Process.h"
#include "contiki-arduino.h"

Process* Process::list = NULL;

PT_THREAD(Process::process_thread_process_manager(struct pt *process_pt, process_event_t ev, process_data_t data))
{
	Process *proc = listFind(PROCESS_CURRENT());
	if (proc)
		return proc->thread(process_pt, ev, data);
	else
		return PT_ENDED;
}

void Process::listAdd(Process *proc) {
	if (list == NULL) {
		list = proc;
	} else {
		Process *prev = list;
		while (prev->_next != NULL)
			prev = prev->_next;
		prev->_next = proc;
	}
}

Process* Process::listFind(struct process *p) {
	Process *q = list;
	for(q = list; &q->_process != p && q != NULL; q = q->_next) ;
	return q;
}

Process::Process(const char *name) {
	_process.next = NULL;
#if !PROCESS_CONF_NO_PROCESS_NAMES
	_process.name = name;
#endif
	_process.thread = process_thread_process_manager;
}

Process::~Process() {
	process_exit(&_process);
}

void Process::run(const char *arg) {
	initialize();
	listAdd(this);
	process_start(&_process, arg);
}
