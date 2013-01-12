/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * SeparateResponse.cpp
 *
 *      Author: smeshlink
 */

#include "SeparateResponse.h"

SeparateResponse::SeparateResponse() : CoAPResponse(&_response, _buffer, NULL), _active(false) {
}

SeparateResponse::~SeparateResponse() {
}

bool SeparateResponse::accept(CoAPRequest& request) {
	if (_active) {
		coap_separate_reject();
		return false;
	} else {
		_active = true;

		/* Take over and skip response by engine. */
		coap_separate_accept(request.request, &_store.request_metadata);
		/* Be aware to respect the Block2 option, which is also stored in the coap_separate_t. */

		/* Restore the request information for the response. */
		coap_separate_resume(&_response, &_store.request_metadata, CONTENT_2_05);

		return true;
	}
}

bool SeparateResponse::send() {
	coap_transaction_t *transaction = NULL;
	if ( (transaction = coap_new_transaction(_store.request_metadata.mid, &_store.request_metadata.addr, _store.request_metadata.port)) ) {
		/* flush buffer into the response packet */
		flush();

		/*
		* Be aware to respect the Block2 option, which is also stored in the coap_separate_t.
		* As it is a critical option, this example resource pretends to handle it for compliance.
		*/
		setBlock2(_store.request_metadata.block2_num, 0, _store.request_metadata.block2_size);

		/* Warning: No check for serialization error. */
		transaction->packet_len = coap_serialize_message(&_response, transaction->packet);
		coap_send_transaction(transaction);
		/* The engine will clear the transaction (right after send for NON, after acked for CON). */

		_active = false;

		return true;
	} else {
		return false;
	}
}
