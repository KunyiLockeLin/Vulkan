#pragma once

#include <stdio.h>
#include "common/common.h"
#include <direct.h>

int main(int argc, char *argv[]) {

    LOG("Create src\\generated_config_struct.h");
    AeFile config_struct;
    config_struct.open("..\\..\\src\\generated_config_struct.h");
    config_struct.addNewLine("#pragma once");
    config_struct.addNewLine("//This file is generated by code_generator.cpp.DO NOT edit this file.");
    config_struct.addNewLine("");

    auto *config = AST->getXML("..\\..\\output\\data\\config.xml");

    const auto *components = config->getXMLNode("components");
    config_struct.addNewLine("enum AE_OBJECT_TYPE {");
    std::string key = "  eAE_OBJECT_";
    for (const auto &comp : components->data->nexts) {
        std::string enum_name = key + comp->data->key + ",";
        config_struct.addNewLine(enum_name.c_str());

    }
    config_struct.addNewLine("};");

    config_struct.close();
    return 0;
}