//
// Created by kca on 25/7/2020.
//

#include "JsonTreeResult.h"
#include "JsonTree.h"

namespace SDV {

// public
JsonTreeResult::
JsonTreeResult()
    : jsonTree{std::make_shared<JsonTree>()}
{}

}  // namespace SDV
