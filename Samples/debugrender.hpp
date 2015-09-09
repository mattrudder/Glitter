#pragma once

#include <glad/glad.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>

// Standard Headers
#include <map>
#include <memory>
#include <vector>

#include "shader.hpp"

struct DebugEntry;
struct DebugVertex;


void DbgInit();
void DbgShutdown();

void DbgBegin();
void DbgEnd();

void DbgSetView(glm::mat4 value);
void DbgSetProjection(glm::mat4 value);
void DbgEnable(bool enable);

void DbgDrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color = glm::vec3(1, 0, 0));
void DbgDrawCoordSystem(glm::mat4 model, float lineLength = 1.0f);
