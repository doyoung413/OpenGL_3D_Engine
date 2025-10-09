#pragma once
#include <string>

enum class ComponentTypes
{
    MESHRENDERER,
    LIGHT,
    INVALID 
};

inline ComponentTypes StringToComponent(const std::string& string)
{
    if (string == "MESHRENDERER")
       return ComponentTypes::MESHRENDERER;
    return ComponentTypes::INVALID;
}

inline std::string ComponentToString(ComponentTypes type)
{
    if (type == ComponentTypes::MESHRENDERER)
        return "MESHRENDERER";
    return "INVALID";
}