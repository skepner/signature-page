#pragma once

#include "acmacs-draw/surface.hh"
#include "rjson-serialize.hh"

// ----------------------------------------------------------------------

class LegendSettings : public rjson::field_container_child
{
 public:
    LegendSettings(rjson::field_container_parent& aParent, std::string aFieldName);

    rjson::field_get_set<Size> offset;  // in enclosing surface
    rjson::field_get_set<double> width; // in enclosing surface scale
    rjson::field_get_set<TextStyle> title_style;
    rjson::field_get_set<double> title_size;
    rjson::field_get_set<TextStyle> text_style;
    rjson::field_get_set<double> text_size;
    rjson::field_get_set<double> interline;

}; // class LegendSettings

// ----------------------------------------------------------------------

class Legend
{
 public:
    inline Legend() {}
    virtual inline ~Legend() = default;

    virtual void draw(Surface& aSurface, const LegendSettings& aSettings) const = 0;
    virtual Size size() const = 0;

}; // class Legend

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
