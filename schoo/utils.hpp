#pragma once

#include<functional>
#include<vulkan/vulkan.hpp>
#include <fstream>
#include <iostream>

namespace schoo{
    std::string ReadWholeFile(const std::string&filename);
}
