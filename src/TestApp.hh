#pragma once

#include "Application.hh"
#include "Loader.hh"
#include <glog/logging.h>

class TestApp : public Application {
SIMPLE_APPLICATION(TestApp, "testapp")
public:
    void init(Loader* loader, const Config& config) override;
};
