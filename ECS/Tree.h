//
// Created by YAHAY on 17/01/2025.
//

#ifndef TREE_H
#define TREE_H
#include <entt/entt.hpp>

struct Tree {
    entt::entity parent;
    entt::entity first_child;
    entt::entity sibling;
};
#endif //TREE_H
