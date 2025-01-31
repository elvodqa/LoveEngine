//
// Created by YAHAY on 20/01/2025.
//

#ifndef INPECTORREPR_H
#define INPECTORREPR_H
#include "../debug_panic.h"

template<typename T>
void inspectorWindow(T* data)
{
    panic("inspector tried to display unsopported type. forgot to specialize?");
}
#endif //INPECTORREPR_H
