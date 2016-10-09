#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ULINK_MAX_FRAME_SIZE 512

struct ulink_frame {
	char buf[ULINK_MAX_FRAME_SIZE]; 
	uint8_t size; 
	uint8_t state; 
	
}; 

int ulink_pack_data(const char *data, size_t size, struct ulink_frame *frame); 
int ulink_parse_frame(const char *data, size_t size, struct ulink_frame *frame); 

void ulink_frame_init(struct ulink_frame *self); 
const char *ulink_frame_data(struct ulink_frame *self); 
size_t ulink_frame_size(struct ulink_frame *self); 
bool ulink_frame_valid(struct ulink_frame *self); 
size_t ulink_frame_to_buffer(struct ulink_frame *self, char *data, size_t max_size); 

#define ulink_stream_each_frame(buffer, buffer_size, frame) \
	for(int _rlen = 0; \
		(_rlen = ulink_parse_frame((buffer) + _rlen, (buffer_size) - _rlen, (frame))) > 0 && ulink_frame_valid(frame); \
		)
