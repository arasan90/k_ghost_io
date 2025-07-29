cmake_minimum_required(VERSION 3.10)

project(k_cjson)

add_library(${PROJECT_NAME} cJSON/cJSON.c cJSON/cJSON_Utils.c)
target_include_directories(${PROJECT_NAME} PUBLIC cJSON)