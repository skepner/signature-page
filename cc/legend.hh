#pragma once

#include "surface.hh"

// ----------------------------------------------------------------------

class Legend
{
 public:
    inline Legend() {}
    virtual inline ~Legend() = default;

    virtual void draw(Surface& aSurface) const = 0;
    virtual Size size() const = 0;

}; // class Legend

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: