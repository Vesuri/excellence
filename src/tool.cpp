#include "tool.h"

Tool::Tool(QObject *parent) : QObject(parent),
    mode_(Paint)
{
}

void Tool::setMode(const Mode &mode)
{
    mode_ = mode;
}
