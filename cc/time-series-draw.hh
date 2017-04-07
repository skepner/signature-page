#pragma once

#include <string>

#include "acmacs-draw/surface.hh"
#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

class Tree;
class TreeDraw;
class HzSection;
class HzSections;

// ----------------------------------------------------------------------

class TimeSeriesDrawSettings
{
 public:
    TimeSeriesDrawSettings();
    ~TimeSeriesDrawSettings();

    std::string begin, end;
    double label_size;
    TextStyle label_style;
    double month_year_to_timeseries_gap;
    Color month_separator_color;
    double month_separator_width;
    double dash_width;
    double dash_line_width;

}; // class TimeSeriesDrawSettings

// ----------------------------------------------------------------------

class TimeSeriesDraw
{
 public:
    inline TimeSeriesDraw(Surface& aSurface, Tree& aTree, const TreeDraw& aTreeDraw, HzSections& aHzSections, TimeSeriesDrawSettings& aSettings)
        : mSurface(aSurface), mTree(aTree), mTreeDraw(aTreeDraw), mSettings(aSettings), mHzSections(aHzSections), mTreeMode(false) {}

    void prepare();
    void draw();

    inline const Size& size() const { return mSurface.viewport().size; }
    inline const Location& origin_in_parent() const { return mSurface.origin_in_parent(); }

    void init_settings();
    inline Surface& surface() { return mSurface; }
    inline void tree_mode(bool aTreeMode) { mTreeMode = aTreeMode; }
    // void hide_hz_section_labels_in_time_series();

 private:
    Surface& mSurface;
    Tree& mTree;
    const TreeDraw& mTreeDraw;
    TimeSeriesDrawSettings& mSettings;
    HzSections& mHzSections;
    Date mBegin, mEnd;
    size_t mNumberOfMonths;
    bool mTreeMode;

    void draw_labels(double month_width);
    void draw_labels_at_side(const Location& aOrigin, double month_width, double month_max_height);
    void draw_month_separators(double month_width);
    void draw_dashes(double month_width);
    void draw_hz_section_lines();
    void draw_hz_section_label(const HzSection& aSection, double aY);

}; // class TimeSeriesDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
