/*
	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "ulink.h"

#define ULINK_FRAME_BYTE 0x7e
#define ULINK_ESCAPE_BYTE 0x7d

enum {
	ULINK_STATE_WAIT, 
	ULINK_STATE_READ, 
	ULINK_STATE_ESCAPE,
	ULINK_STATE_DONE
}; 

int ulink_pack_data(const char *data, size_t size, struct ulink_frame *frame){
	frame->size = 0; 
	char *packet_buf = frame->buf; 
	size_t c; 
	for(c = 0; c < size; c++){
		uint8_t ch = data[c];  
		switch(ch){
			case ULINK_FRAME_BYTE:
			case ULINK_ESCAPE_BYTE: {
				*packet_buf++ = ULINK_ESCAPE_BYTE; frame->size++;  
				if(frame->size == ULINK_MAX_FRAME_SIZE) break;  
				*packet_buf++ = ch ^ 0x20; frame->size++;  
				if(frame->size == ULINK_MAX_FRAME_SIZE) break; 
			} break; 
			default:  {	
				*packet_buf++ = ch; frame->size++;  
				if(frame->size == ULINK_MAX_FRAME_SIZE) break;  
			} break; 
		}
	}
	*packet_buf++ = ULINK_FRAME_BYTE; frame->size++;  
	return c; 
}

void ulink_frame_init(struct ulink_frame *self){
	self->size = 0; 
	self->state = ULINK_STATE_READ; 
}

bool ulink_frame_valid(struct ulink_frame *self){
	return self->state == ULINK_STATE_DONE && self->size > 0; 
}

const char *ulink_frame_data(struct ulink_frame *self){
	return self->buf; 
}

size_t ulink_frame_size(struct ulink_frame *self){
	return self->size; 
}

size_t ulink_frame_to_buffer(struct ulink_frame *self, char *data, size_t size){
	size_t s = self->size; 
	if(size < s) s = size; 
	memcpy(data, self->buf, s); 
	return s; 
}

static uint16_t _ulink_checksum(const char *packet, size_t size){
	return 0; 
}

int ulink_parse_frame(const char *packet, size_t size, struct ulink_frame *frame){
	if(frame->state == ULINK_STATE_DONE) {
		frame->state = ULINK_STATE_READ; 
		frame->size = 0; 
	}
	for(size_t c = 0; c < size; c++){
		uint8_t byte = *packet++; 
		switch(byte){
			case ULINK_FRAME_BYTE: {
				// new frame so we validate the data in the parsed buffer and return number of bytes processed
				_ulink_checksum(packet, size); 
				frame->state = ULINK_STATE_DONE; 
				return c; 
			} break; 
			case ULINK_ESCAPE_BYTE: {
				frame->state = ULINK_STATE_ESCAPE; 
			} break; 
			default: {
				switch(frame->state){
					case ULINK_STATE_READ: {
						frame->buf[frame->size++] = byte; 
					} break; 
					case ULINK_STATE_ESCAPE: {
						// only 0x5d and 0x5e are valid escaped chars
						if(byte != 0x5d && byte != 0x5e) goto error; 
						frame->buf[frame->size++] = byte ^ 0x20; 
						frame->state = ULINK_STATE_READ; 
					} break; 
				}
				if(frame->size == ULINK_MAX_FRAME_SIZE) goto error; 
			} break; 
		}
	}
	return size; 
error: 
	// we should never reach this point so we reset the buffer and start over
	frame->size = 0; 
	frame->state = ULINK_STATE_READ; 
	return -EAGAIN; 
}


