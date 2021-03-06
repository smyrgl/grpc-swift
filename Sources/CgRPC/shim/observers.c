/*
 *
 * Copyright 2016, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "internal.h"
#include "cgrpc.h"

#include <stdlib.h>
#include <string.h>

// create observers for each type of GRPC operation

cgrpc_observer_send_initial_metadata *cgrpc_observer_create_send_initial_metadata(cgrpc_metadata_array *metadata) {
  cgrpc_observer_send_initial_metadata *observer =
  (cgrpc_observer_send_initial_metadata *) malloc(sizeof (cgrpc_observer_send_initial_metadata));
  observer->op_type = GRPC_OP_SEND_INITIAL_METADATA;
  cgrpc_metadata_array_move_metadata(&(observer->initial_metadata), metadata);
  return observer;
}

cgrpc_observer_send_message *cgrpc_observer_create_send_message() {
  cgrpc_observer_send_message *observer =
  (cgrpc_observer_send_message *) malloc(sizeof (cgrpc_observer_send_message));
  observer->op_type = GRPC_OP_SEND_MESSAGE;
  return observer;
}

cgrpc_observer_send_close_from_client *cgrpc_observer_create_send_close_from_client() {
  cgrpc_observer_send_close_from_client *observer =
  (cgrpc_observer_send_close_from_client *) malloc(sizeof (cgrpc_observer_send_close_from_client));
  observer->op_type = GRPC_OP_SEND_CLOSE_FROM_CLIENT;
  return observer;
}

cgrpc_observer_send_status_from_server *cgrpc_observer_create_send_status_from_server(cgrpc_metadata_array *metadata) {
  cgrpc_observer_send_status_from_server *observer =
  (cgrpc_observer_send_status_from_server *) malloc(sizeof (cgrpc_observer_send_status_from_server));
  observer->op_type = GRPC_OP_SEND_STATUS_FROM_SERVER;
  cgrpc_metadata_array_move_metadata(&(observer->trailing_metadata), metadata);
  return observer;
}

cgrpc_observer_recv_initial_metadata *cgrpc_observer_create_recv_initial_metadata() {
  cgrpc_observer_recv_initial_metadata *observer =
  (cgrpc_observer_recv_initial_metadata *) malloc(sizeof (cgrpc_observer_recv_initial_metadata));
  observer->op_type = GRPC_OP_RECV_INITIAL_METADATA;
  return observer;
}

cgrpc_observer_recv_message *cgrpc_observer_create_recv_message() {
  cgrpc_observer_recv_message *observer =
  (cgrpc_observer_recv_message *) malloc(sizeof (cgrpc_observer_recv_message));
  observer->op_type = GRPC_OP_RECV_MESSAGE;
  observer->response_payload_recv = NULL;
  return observer;
}

cgrpc_observer_recv_status_on_client *cgrpc_observer_create_recv_status_on_client() {
  cgrpc_observer_recv_status_on_client *observer =
  (cgrpc_observer_recv_status_on_client *) malloc(sizeof (cgrpc_observer_recv_status_on_client));
  observer->op_type = GRPC_OP_RECV_STATUS_ON_CLIENT;
  return observer;
}

cgrpc_observer_recv_close_on_server *cgrpc_observer_create_recv_close_on_server() {
  cgrpc_observer_recv_close_on_server *observer =
  (cgrpc_observer_recv_close_on_server *) malloc(sizeof (cgrpc_observer_recv_close_on_server));
  observer->op_type = GRPC_OP_RECV_CLOSE_ON_SERVER;
  observer->was_cancelled = 0;
  return observer;
}

// apply observer to operation

void cgrpc_observer_apply(cgrpc_observer *observer, grpc_op *op) {
  op->op = observer->op_type;
  op->flags = 0;
  op->reserved = NULL;

  switch (observer->op_type) {
    case GRPC_OP_SEND_INITIAL_METADATA: {
      cgrpc_observer_send_initial_metadata *obs = (cgrpc_observer_send_initial_metadata *) observer;
      op->data.send_initial_metadata.count = obs->initial_metadata.count;
      op->data.send_initial_metadata.metadata = obs->initial_metadata.metadata;
      break;
    }
    case GRPC_OP_SEND_MESSAGE: {
      cgrpc_observer_send_message *obs = (cgrpc_observer_send_message *) observer;
      op->data.send_message = obs->request_payload;
      break;
    }
    case GRPC_OP_SEND_CLOSE_FROM_CLIENT: {
      break;
    }
    case GRPC_OP_SEND_STATUS_FROM_SERVER: {
      cgrpc_observer_send_status_from_server *obs = (cgrpc_observer_send_status_from_server *) observer;
      op->data.send_status_from_server.trailing_metadata_count = obs->trailing_metadata.count;
      op->data.send_status_from_server.trailing_metadata = obs->trailing_metadata.metadata;
      op->data.send_status_from_server.status = obs->status;
      op->data.send_status_from_server.status_details = obs->status_details;
      break;
    }
    case GRPC_OP_RECV_INITIAL_METADATA: {
      cgrpc_observer_recv_initial_metadata *obs = (cgrpc_observer_recv_initial_metadata *) observer;
      grpc_metadata_array_init(&(obs->initial_metadata_recv));
      op->data.recv_initial_metadata = &(obs->initial_metadata_recv);
      break;
    }
    case GRPC_OP_RECV_MESSAGE: {
      cgrpc_observer_recv_message *obs = (cgrpc_observer_recv_message *) observer;
      op->data.recv_message = &(obs->response_payload_recv);
      break;
    }
    case GRPC_OP_RECV_STATUS_ON_CLIENT: {
      cgrpc_observer_recv_status_on_client *obs = (cgrpc_observer_recv_status_on_client *) observer;
      grpc_metadata_array_init(&(obs->trailing_metadata_recv));
      obs->server_status = GRPC_STATUS_OK;
      obs->server_details = NULL;
      obs->server_details_capacity = 0;
      op->data.recv_status_on_client.trailing_metadata = &(obs->trailing_metadata_recv);
      op->data.recv_status_on_client.status = &(obs->server_status);
      op->data.recv_status_on_client.status_details = &(obs->server_details);
      op->data.recv_status_on_client.status_details_capacity = &(obs->server_details_capacity);
      break;
    }
    case GRPC_OP_RECV_CLOSE_ON_SERVER: {
      cgrpc_observer_recv_close_on_server *obs = (cgrpc_observer_recv_close_on_server *) observer;
      op->data.recv_close_on_server.cancelled = &(obs->was_cancelled);
      break;
    }
    default: {

    }
  }
}

// destroy all observers

void cgrpc_observer_destroy(cgrpc_observer *observer) {
  switch (observer->op_type) {
    case GRPC_OP_SEND_INITIAL_METADATA: {
      cgrpc_observer_send_initial_metadata *obs = (cgrpc_observer_send_initial_metadata *) observer;
      grpc_metadata_array_destroy(&(obs->initial_metadata));
      free(obs);
      break;
    }
    case GRPC_OP_SEND_MESSAGE: {
      cgrpc_observer_send_message *obs = (cgrpc_observer_send_message *) observer;
      grpc_byte_buffer_destroy(obs->request_payload);
      free(obs);
      break;
    }
    case GRPC_OP_SEND_CLOSE_FROM_CLIENT: {
      cgrpc_observer_send_close_from_client *obs = (cgrpc_observer_send_close_from_client *) observer;
      free(obs);
      break;
    }
    case GRPC_OP_SEND_STATUS_FROM_SERVER: {
      cgrpc_observer_send_status_from_server *obs = (cgrpc_observer_send_status_from_server *) observer;
      free(obs);
      break;
    }
    case GRPC_OP_RECV_INITIAL_METADATA: {
      cgrpc_observer_recv_initial_metadata *obs = (cgrpc_observer_recv_initial_metadata *) observer;
      grpc_metadata_array_destroy(&obs->initial_metadata_recv);
      free(obs);
      break;
    }
    case GRPC_OP_RECV_MESSAGE: {
      cgrpc_observer_recv_message *obs = (cgrpc_observer_recv_message *) observer;
      grpc_byte_buffer_destroy(obs->response_payload_recv);
      free(obs);
      break;
    }
    case GRPC_OP_RECV_STATUS_ON_CLIENT: {
      cgrpc_observer_recv_status_on_client *obs = (cgrpc_observer_recv_status_on_client *) observer;
      free(obs->server_details);
      grpc_metadata_array_destroy(&(obs->trailing_metadata_recv));
      free(obs);
      break;
    }
    case GRPC_OP_RECV_CLOSE_ON_SERVER: {
      cgrpc_observer_recv_close_on_server *obs = (cgrpc_observer_recv_close_on_server *) observer;
      free(obs);
      break;
    }
    default: {

    }
  }
}

cgrpc_byte_buffer *cgrpc_observer_recv_message_get_message(cgrpc_observer_recv_message *observer) {
  if (observer->response_payload_recv) {
    return grpc_byte_buffer_copy(observer->response_payload_recv);
  } else {
    return NULL;
  }
}

cgrpc_metadata_array *cgrpc_observer_recv_initial_metadata_get_metadata(cgrpc_observer_recv_initial_metadata *observer) {
  cgrpc_metadata_array *metadata = cgrpc_metadata_array_create();
  cgrpc_metadata_array_move_metadata(metadata, &(observer->initial_metadata_recv));
  return metadata;
}

void cgrpc_observer_send_message_set_message(cgrpc_observer_send_message *observer, cgrpc_byte_buffer *message) {
  observer->request_payload = grpc_byte_buffer_copy(message);
}

void cgrpc_observer_send_status_from_server_set_status(cgrpc_observer_send_status_from_server *observer, int status) {
  observer->status = status;
}

void cgrpc_observer_send_status_from_server_set_status_details(cgrpc_observer_send_status_from_server *observer, const char *statusDetails) {
  observer->status_details = strdup(statusDetails);
}

cgrpc_metadata_array *cgrpc_observer_recv_status_on_client_get_metadata(cgrpc_observer_recv_status_on_client *observer) {
  cgrpc_metadata_array *metadata = cgrpc_metadata_array_create();
  cgrpc_metadata_array_move_metadata(metadata, &(observer->trailing_metadata_recv));
  return metadata;
}

long cgrpc_observer_recv_status_on_client_get_status(cgrpc_observer_recv_status_on_client *observer) {
  return observer->server_status;
}

const char *cgrpc_observer_recv_status_on_client_get_status_details(cgrpc_observer_recv_status_on_client *observer) {
  return observer->server_details;
}

int cgrpc_observer_recv_close_on_server_get_was_cancelled(cgrpc_observer_recv_close_on_server *observer) {
  return observer->was_cancelled;
}

