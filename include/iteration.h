#ifndef ITERATION_H_
#define ITERATION_H_

// Iteration decisions for *.for_each_*(...)
// type member functions.
//
// "return ITERATION::CONTINUE;" to continue
// "return ITERATION::BREAK;" to break
namespace ITERATION {
enum STATEMENT {
    CONTINUE,
    BREAK,
};
}

#endif
