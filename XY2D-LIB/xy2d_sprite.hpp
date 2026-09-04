#pragma once
#ifndef __XY2D_SPRITE
#define __XY2D_SPRITE
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_sprite {
		public:
			xy2d_vertex vertices[6] = {
				{ glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
				{ glm::vec3(1.0, 0.0, 0.0), glm::vec2(1.0, 0.0) },
				{ glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 1.0) },
				{ glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
				{ glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 1.0) },
				{ glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0, 1.0) },
			};
			
			glm::int32_t batchIndex = -1.0;
			glm::vec4 xywh = glm::vec4(0.0, 0.0, 1.0, 1.0);
			glm::vec4 uvwh = glm::vec4(0.0, 0.0, 1.0, 1.0);
			glm::vec2 xyscale = glm::vec2(1.0, 1.0);
			glm::vec2 xyorigin = glm::vec2(0.0, 0.0);
			glm::float32_t depth = 0.0;
			glm::float32_t theta = 0.0;
			
			xy2d_sprite() {};
			xy2d_sprite(glm::vec4& xywh, glm::vec4& uvwh, glm::vec2& xyscale, glm::vec2& xyorigin, glm::float32_t& depth, glm::float32_t& theta)
				: xywh(xywh), uvwh(uvwh), xyscale(xyscale), xyorigin(xyorigin), depth(depth), theta(theta) { Update(); };
			
			xy2d_sprite& PosSize(glm::vec4 xywh) { this->xywh = xywh; return (*this); }
			xy2d_sprite& Position(glm::vec2 xy) { this->xywh.x = xy.x; this->xywh.y = xy.y; return (*this); }
			xy2d_sprite& Size(glm::vec2 wh) { this->xywh.z = wh.x; this->xywh.w = wh.y; return (*this); }
			xy2d_sprite& Origin(glm::vec2 xyorigin) { this->xyorigin = xyorigin; return (*this); }
			xy2d_sprite& Scale(glm::vec2 xyscale) { this->xyscale = xyscale; return (*this); }
			xy2d_sprite& Rotate(glm::float32 theta) { this->theta = theta; return (*this); }
			xy2d_sprite& Texture(glm::vec4 uvwh) { this->uvwh = uvwh; return (*this); }
			
			xy2d_sprite& Update() {
				vertices[0] = { glm::vec3(xywh.x         , xywh.y         , depth), glm::vec2(uvwh.x         , uvwh.y         ) };
				vertices[1] = { glm::vec3(xywh.x + xywh.z, xywh.y         , depth), glm::vec2(uvwh.x + uvwh.z, uvwh.y         ) };
				vertices[4] = { glm::vec3(xywh.x + xywh.z, xywh.y + xywh.w, depth), glm::vec2(uvwh.x + uvwh.z, uvwh.y + uvwh.w) };
				vertices[5] = { glm::vec3(xywh.x         , xywh.y + xywh.w, depth), glm::vec2(uvwh.x         , uvwh.y + uvwh.w) };
				
				glm::mat2 rotmatrix = glm::mat2(glm::cos(theta), -glm::sin(theta), glm::sin(theta), glm::cos(theta));
				
				for(size_t i = 0, corner[4] = { 0, 1, 4, 5 }; i < std::size(corner); i++) {
					glm::vec2 xypos = glm::vec2(vertices[corner[i]].xyz);
					xypos -= glm::vec2(xywh);
					xypos = rotmatrix * ((xypos - glm::vec2(xyorigin.x, xyorigin.y)) * xyscale);
					xypos += glm::vec2(xywh);
					vertices[corner[i]].xyz = glm::vec3(xypos, depth);
				}
				
				vertices[2] = vertices[4], vertices[3] = vertices[0];
				return (*this);
			}
			
			size_t SizeOf() {
				return sizeof(vertices);
			}
			
			inline static xy2d_sprite CreateSprite(glm::vec4 xywh, glm::vec4 uvwh, glm::vec2 xyscale, glm::vec2 xyorigin, glm::float32_t depth, glm::float32_t theta) {
				return xy2d_sprite(xywh, uvwh, xyscale, xyorigin, depth, theta);
			}
			
			inline static glm::vec4 GetUVCoords(glm::vec4 xywh, glm::vec2 textsize) {
				return glm::vec4(xywh.x, xywh.y, xywh.x + xywh.z, xywh.y + xywh.w) / glm::vec4(textsize.x, textsize.y, textsize.x, textsize.y);
			}
		};
		
		class xy2d_sprite_manager {
		public:
			inline static std::vector<xy2d_sprite*> spriteBatch;
			inline static std::vector<xy2d_vertex*> vertexBatch;
			
			inline static void BatchSprite(xy2d_sprite& sprite) {
				sprite.batchIndex = spriteBatch.size();
				spriteBatch.push_back(&sprite);
				
				for(uint32_t i = 0; i < sizeof(sprite.vertices); i++)
					vertexBatch.push_back(&sprite.vertices[i]);
			}
			
			inline static void BatchSpriteList(const std::vector<std::reference_wrapper<xy2d_sprite>>& spriteList) {
				for(xy2d_sprite& sprite : spriteList) {
					sprite.batchIndex = spriteBatch.size();
					spriteBatch.push_back(&sprite);
				}
			}
			
			inline static void BatchDepthSort() {
				std::sort(spriteBatch.begin(), spriteBatch.end(), [](xy2d_sprite* A, xy2d_sprite* B) { return A->depth < B->depth; });
			}
			
			inline static size_t BatchVerticesSize() {
				return (spriteBatch.size() > 0)? sizeof(spriteBatch[0]->vertices) * spriteBatch.size() : 0;
			}
		};
	}
#endif