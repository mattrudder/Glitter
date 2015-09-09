#pragma once

#include <string>
#include <functional>

struct Sample {
  typedef std::function<void* ()> LoadFunc;
  typedef std::function<void(void*)> UnloadFunc;
  typedef std::function<void(void*)> RenderFunc;

  Sample(std::string const & name, LoadFunc load, UnloadFunc unload, RenderFunc render);

  std::string mName;
  LoadFunc mLoad;
  UnloadFunc mUnload;
  RenderFunc mRender;

  static Sample* getSample(size_t i);
  static size_t getSampleCount();
};