#ifndef UISTATE_H
#define UISTATE_H

class AbstractStateBehavior;

struct FilesState
{
    bool gcodeOpened = false;
    bool heightmapOpened = false;
};

struct UiState
{
    AbstractStateBehavior* sb = nullptr;
    FilesState files;
};

#endif // UISTATE_H
