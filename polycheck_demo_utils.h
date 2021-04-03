//
// Created by behrooz on 2021-04-02.
//

#ifndef PROJECT_POLYCHECK_DEMO_UTILS_H
#define PROJECT_POLYCHECK_DEMO_UTILS_H
#include <rose.h>

class OriginalSchedule: public AstSimpleProcessing
{
public:
    OriginalSchedule();
    virtual void visit(SgNode *node);
};


#endif //PROJECT_POLYCHECK_DEMO_UTILS_H
