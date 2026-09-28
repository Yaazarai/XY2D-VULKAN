#pragma once
#ifndef __XY2D_SPRITE
#define __XY2D_SPRITE
	#include ".\xy2d_engine.hpp"
	
	struct xy2d_sprite {
		xy2d_vertex vertices[6] = {
			{ glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::uint32(0xFFFFFFFFU) },
			{ glm::vec2(1.0, 0.0), glm::vec2(1.0, 0.0), glm::uint32(0xFFFFFFFFU) },
			{ glm::vec2(1.0, 1.0), glm::vec2(1.0, 1.0), glm::uint32(0xFFFFFFFFU) },
			{ glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), glm::uint32(0xFFFFFFFFU) },
			{ glm::vec2(1.0, 1.0), glm::vec2(1.0, 1.0), glm::uint32(0xFFFFFFFFU) },
			{ glm::vec2(0.0, 1.0), glm::vec2(0.0, 1.0), glm::uint32(0xFFFFFFFFU) },
		};
		
		glm::int32_t batchIndex = -1.0;
		glm::vec4 xywh = glm::vec4(0.0, 0.0, 1.0, 1.0);
		glm::vec4 uvwh = glm::vec4(0.0, 0.0, 1.0, 1.0);
		glm::vec2 xyscale = glm::vec2(1.0, 1.0);
		glm::vec2 xyorigin = glm::vec2(0.0, 0.0);
		glm::float32 theta = 0.0;
		glm::float32 depth = 0.0;
		glm::uint32 color = 0xFFFFFFFFU;
	};
	
	xy2d_sprite& xy2d_sprite_position(xy2d_sprite& sprite, glm::vec2 xy) { sprite.xywh.x = xy.x; sprite.xywh.y = xy.y; return sprite; }
	xy2d_sprite& xy2d_sprite_size(xy2d_sprite& sprite, glm::vec2 wh) { sprite.xywh.z = wh.x; sprite.xywh.w = wh.y; return sprite; }
	xy2d_sprite& xy2d_sprite_origin(xy2d_sprite& sprite, glm::vec2 xyorigin) { sprite.xyorigin = xyorigin; return sprite; }
	xy2d_sprite& xy2d_sprite_scale(xy2d_sprite& sprite, glm::vec2 xyscale) { sprite.xyscale = xyscale; return sprite; }
	xy2d_sprite& xy2d_sprite_rotate(xy2d_sprite& sprite, glm::float32 theta) { sprite.theta = theta; return sprite; }
	xy2d_sprite& xy2d_sprite_texture(xy2d_sprite& sprite, glm::vec4 uvwh) { sprite.uvwh = uvwh; return sprite; }
	xy2d_sprite& xy2d_sprite_color(xy2d_sprite& sprite, glm::uint32 col) { sprite.color = col; return sprite; }
	xy2d_sprite& xy2d_sprite_depth(xy2d_sprite& sprite, glm::float32 dpt) { sprite.depth = dpt; return sprite; }
	
	xy2d_sprite& xy2d_sprite_update(xy2d_sprite& sprite) {
		sprite.vertices[0] = { glm::vec2(sprite.xywh.x, sprite.xywh.y), glm::vec2(sprite.uvwh.x, sprite.uvwh.y), sprite.color };
		sprite.vertices[1] = { glm::vec2(sprite.xywh.x + sprite.xywh.z, sprite.xywh.y), glm::vec2(sprite.uvwh.x + sprite.uvwh.z, sprite.uvwh.y), sprite.color };
		sprite.vertices[4] = { glm::vec2(sprite.xywh.x + sprite.xywh.z, sprite.xywh.y + sprite.xywh.w), glm::vec2(sprite.uvwh.x + sprite.uvwh.z, sprite.uvwh.y + sprite.uvwh.w), sprite.color };
		sprite.vertices[5] = { glm::vec2(sprite.xywh.x, sprite.xywh.y + sprite.xywh.w), glm::vec2(sprite.uvwh.x, sprite.uvwh.y + sprite.uvwh.w), sprite.color };
		
		glm::mat2 rotmatrix = glm::mat2(glm::cos(sprite.theta), -glm::sin(sprite.theta), glm::sin(sprite.theta), glm::cos(sprite.theta));
		for(size_t i = 0, corner[4] = { 0, 1, 4, 5 }; i < std::size(corner); i++) {
			glm::vec2 xypos = glm::vec2(sprite.vertices[corner[i]].xy);
			xypos -= glm::vec2(sprite.xywh);
			xypos = rotmatrix * ((xypos - sprite.xyorigin) * sprite.xyscale);
			xypos += glm::vec2(sprite.xywh);
			sprite.vertices[corner[i]].xy = xypos;
		}
		
		sprite.vertices[2] = sprite.vertices[4], sprite.vertices[3] = sprite.vertices[0];
		return sprite;
	}
	
	size_t xy2d_sprite_sizeof() {
		return sizeof(xy2d_sprite::vertices);
	}
	
	xy2d_sprite xy2d_sprite_create(glm::vec4 xywh, glm::vec4 uvwh, glm::vec2 xyscale, glm::vec2 xyorigin, glm::float32 theta = 0.0f, glm::float32 depth = 0.0f, glm::uint32 color = 0xFFFFFFFFU) {
		return { .xywh = xywh, .uvwh = uvwh, .xyscale = xyscale, .xyorigin = xyorigin, .theta = theta, .depth = depth, .color = color };
	}
	
	glm::vec4 xy2d_sprite_uvcoords(glm::vec4 xywh, glm::vec2 textsize) {
		return glm::vec4(xywh.x, xywh.y, xywh.x + xywh.z, xywh.y + xywh.w) / glm::vec4(textsize.x, textsize.y, textsize.x, textsize.y);
	}
#endif