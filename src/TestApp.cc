#include "TestApp.hh"

REGISTER_APPLICATION(TestApp, {""})

void TestApp::init(Loader *loader, const Config& config)
{
    LOG(INFO) << "Hello, world!";
}