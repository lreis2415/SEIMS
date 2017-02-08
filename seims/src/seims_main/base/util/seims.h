#ifndef SEIMS_HEADER
#define SEIMS_HEADER
/*!
 * \enum LayeringMethod
 * \brief Grid layering method for routing and parallel computing
 */
enum LayeringMethod
{
    /// layering-from-source method
    UP_DOWN,
    /// layering-from-outlet method
    DOWN_UP
};
#endif
