/// @brief Source: https://stackoverflow.com/questions/9568150/what-is-a-c-delegate/9568485#9568485
#pragma once
#ifndef __XY2D_CAMERA
#define __XY2D_CAMERA
	#include "./xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_camera {
		public:
			glm::mat4 projection;
			glm::vec4 xrotation;
			glm::vec4 yrotation;
			glm::vec2 position;
			glm::vec2 scale;
			glm::vec2 origin;
			
			xy2d_camera(glm::mat4 projection, glm::vec4 xrot, glm::vec4 yrot, glm::vec2 position, glm::vec2 scale, glm::vec2 origin)
			: projection(projection), xrotation(xrot), yrotation(yrot), position(position), scale(scale), origin(origin) {}
			
			inline static xy2d_camera GetCamera(glm::vec2 dimensions, glm::vec2 position, glm::vec2 scale, glm::vec2 origin, glm::float32 theta, glm::float32 znear = 1.0, glm::float32 zfar = -1.0) {
				glm::mat4 prj = glm::ortho(0.0f, dimensions.x, 0.0f, dimensions.y, znear, zfar);
				glm::vec4 xrot = glm::vec4(glm::cos(theta), -glm::sin(theta), 0.0, 0.0);
				glm::vec4 yrot = glm::vec4(glm::sin(theta), glm::cos(theta), 0.0, 0.0);
				return xy2d_camera(prj, xrot, yrot, position, scale, origin);
			}
		};
	}
#endif