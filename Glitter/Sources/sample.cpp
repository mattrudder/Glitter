#include "sample.hpp"

#include <vector>

std::vector<Sample*> gSamples;

Sample::Sample(std::string const & name, LoadFunc load, UnloadFunc unload, RenderFunc render)
  : mName(name)
  , mLoad(load)
  , mUnload(unload)
  , mRender(render) {
  gSamples.push_back(this);
}

size_t Sample::getSampleCount() {
  return gSamples.size();
}

Sample* Sample::getSample(size_t i) {
  return i < gSamples.size() ? gSamples[i] : nullptr;
}
