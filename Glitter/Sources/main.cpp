// Preprocessor Directives
#define STB_IMAGE_IMPLEMENTATION

// Local Headers
#include "glitter.hpp"
#include "imgui_impl_glfw_gl3.hpp"
#include "sample.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

size_t gSampleIndex = 0;
Sample* gSample = nullptr;
void* gSampleState = nullptr;

void loadSample(size_t index, bool reload = false) {
  if (!gSample)
    reload = true;
  if (index >= Sample::getSampleCount())
    index = 0;
  if (index == gSampleIndex && !reload)
    return;

  Sample* sample = Sample::getSample(index);
  if (sample) {
    if (gSample) {
      gSample->mUnload(gSampleState);
    }
    gSampleIndex = index;
    gSample = sample;
    gSampleState = sample->mLoad();
  }
}

int main(int argc, char * argv[]) {
  // Load GLFW and Create a Window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

  // Check for Valid Context
  if (mWindow == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    return EXIT_FAILURE;
  }

  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(mWindow);
  gladLoadGL();
  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

  ImGui_ImplGlfwGL3_Init(mWindow, true);
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF(PROJECT_SOURCE_DIR "/Glitter/Fonts/FiraMono-Regular.ttf", 14.0f, NULL, io.Fonts->GetGlyphRangesDefault());

  // Rendering Loop
  while (glfwWindowShouldClose(mWindow) == false) {
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(mWindow, true);

    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();

    if (gSample == nullptr) loadSample(0);

    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("Samples")) {
        auto sampleCount = Sample::getSampleCount();
        for (size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
          auto sample = Sample::getSample(sampleIndex);
          if (ImGui::MenuItem(sample->mName.c_str(), "", sampleIndex == gSampleIndex)) {
            loadSample(sampleIndex);
          }
        }

        if (sampleCount > 0) {
          ImGui::Separator();

          if (ImGui::MenuItem("Previous Sample", "CTRL+[", false)) {
            loadSample(gSampleIndex == 0 ? Sample::getSampleCount() - 1 : gSampleIndex - 1);
          }
          if (ImGui::MenuItem("Next Sample", "CTRL+]", false)) {
            loadSample(gSampleIndex + 1);
          }

          if (ImGui::MenuItem("Reload Sample", "CTRL+R", false)) {
            loadSample(gSampleIndex, true);
          }
        }

        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if (gSample)
      gSample->mRender(gSampleState);

    ImGui::Render();

    // Flip Buffers and Draw
    glfwSwapBuffers(mWindow);
  }

  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();
  return EXIT_SUCCESS;
}
