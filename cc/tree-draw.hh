#pragma once

#include <string>
#include <limits>
#include <utility>
#include <memory>
#include <algorithm>

#include "surface.hh"
#include "legend.hh"
#include "clades-draw.hh"

// ----------------------------------------------------------------------

class Node;
class Tree;
class Coloring;

// ----------------------------------------------------------------------

class AATransitionPerBranchDrawSettings
{
 public:
    inline AATransitionPerBranchDrawSettings()
        : size(20), color("black"), style("Courier New"), interline(1.2),
          label_offset{-40, 20}, label_connection_line_width(0.1), label_connection_line_color("black") {}

    double size;
    Color color;
    TextStyle style;
    double interline;
    Size label_offset;
    double label_connection_line_width;
    Color label_connection_line_color;

}; // class AATransitionPerBranchDrawSettings

// ----------------------------------------------------------------------

class AATransitionDrawSettings
{
 public:
    inline AATransitionDrawSettings()
        : show(true), number_strains_threshold(20), show_empty_left(false), show_node_for_left_line(false),
          node_for_left_line_color("green"), node_for_left_line_width(1)
        {}

    bool show;
    size_t number_strains_threshold; // Do not show aa transition label if number_strains (leaf nodes) for the branch is less than this value.
    bool show_empty_left;
    AATransitionPerBranchDrawSettings per_branch;
    bool show_node_for_left_line;
    Color node_for_left_line_color;
    double node_for_left_line_width;

}; // class AATransitionDrawSettings

// ----------------------------------------------------------------------

class TreeDrawVaccineSettings
{
 public:
    inline TreeDrawVaccineSettings()
        : label_color("black"), label_size(20), line_color("black"), line_width(1) {}

    std::string name;           // empty for default settings
    Color label_color;
    double label_size;
    TextStyle label_style;
    Color line_color;
    double line_width;

}; // class TreeDrawVaccineSettings

// ----------------------------------------------------------------------

class TreeDrawSettings
{
 public:
    inline TreeDrawSettings()
        : hide_if_cumulative_edge_length_bigger_than(0.05), //(std::numeric_limits<double>::max()),
          force_line_width(false),
          line_width(1), root_edge(0), line_color(0), name_offset(0.3), color_nodes("continent"),
          vaccines{{TreeDrawVaccineSettings{}}}
        {}
    ~TreeDrawSettings();

    inline const TreeDrawVaccineSettings& vaccine(std::string aName) const
        {
            auto p = std::find_if(vaccines.begin(), vaccines.end(), [&aName](const auto& e) { return e.name == aName; });
            if (p == vaccines.end())
                p = std::find_if(vaccines.begin(), vaccines.end(), [](const auto& e) { return e.name.empty(); });
            if (p == vaccines.end())
                throw std::runtime_error("Invalid tree.vaccines settings: neither default entry (with empty name) nor entry with name " + aName);
            return *p;
        }

    std::string root;           // re-root tree
    std::string hide_isolated_before; // hide leaves isolated before the date (empty -> do not hide based on date)
    double hide_if_cumulative_edge_length_bigger_than; // hide long branches
    bool force_line_width;
    double line_width;
    double root_edge;
    Color line_color;
    TextStyle label_style;
    double name_offset;         // offset of the label from the line right end, in W widths
    std::string color_nodes;    // black, continent, position number (e.g. 162)
    AATransitionDrawSettings aa_transition;
    std::vector<TreeDrawVaccineSettings> vaccines;
    LegendSettings legend;

}; // class TreeDrawSettings

// ----------------------------------------------------------------------

class HzSection
{
 public:
    inline HzSection(std::string aName = std::string{}, bool aShowLine = true)
        : show(true), show_line(aShowLine), show_label_in_time_series(true), show_map(true), name(aName), first(nullptr), last(nullptr), index(1, '?') {}
    inline HzSection(const HzSection&) = default;
    // inline HzSection(HzSection&&) = default;
    ~HzSection();
    inline HzSection& operator=(const HzSection&) = default;

    bool show;
    bool show_line;
    bool show_label_in_time_series;
    bool show_map;
    std::string name;           // first seq_id
    std::string label;          // antigenic map label, empty - generate automatically

    // not stored in settings
    const Node* first;
    const Node* last;
    std::string index;
};

class HzSections
{
 public:
    inline HzSections()
        : vertical_gap(50), line_color("grey63"), line_width(10), ts_label_size(50), ts_label_color("black") {}
    ~HzSections();

    void sort(const Tree& aTree);
    void auto_detect(Tree& aTree, const Clades* aClades);

    inline size_t shown_maps() const
        {
            return std::accumulate(sections.begin(), sections.end(), 0U, [](size_t a, const HzSection& section) -> size_t { return a + (section.show_map ? 1 : 0); });
        }

    double vertical_gap;
    Color line_color;
    double line_width;
    double ts_label_size;
    TextStyle ts_label_style;
    Color ts_label_color;
    std::vector<HzSection> sections;
};

// ----------------------------------------------------------------------

class TreeDraw
{
 public:
    TreeDraw(Surface& aSurface, Tree& aTree, TreeDrawSettings& aSettings, HzSections& aHzSections);
    ~TreeDraw();

    void prepare();
    void draw();

    const Legend* coloring_legend() const;
    const Coloring& coloring() const { return *mColoring; }
    double vertical_step() const { return mVerticalStep; }

    void init_settings(const Clades* aClades);
    void set_line_no(bool aForce, bool aHideLeaves);
    inline Surface& surface() { return mSurface; }

 private:
    Surface& mSurface;
    Tree& mTree;
    TreeDrawSettings& mSettings;
    HzSections& mHzSections;
    std::unique_ptr<Coloring> mColoring;
    mutable std::unique_ptr<Legend> mColoringLegend;

    double mHorizontalStep;
    double mVerticalStep;
    double mLineWidth;
    double mFontSize;
    double mNameOffset;
    bool hiding_leaves_done;
    bool setting_line_no_done;

    void hide_leaves(bool aForce);
    void set_vertical_pos();
    size_t prepare_hz_sections();
    void draw_node(const Node& aNode, double aOriginX, double& aVerticalGap, double aEdgeLength = -1);
    void draw_legend();
    void draw_aa_transition(const Node& aNode, const Location& aOrigin, double aRight);
    void fit_labels_into_viewport();
    void calculate_name_offset();

    inline double text_width(std::string text) { return mSurface.text_size(text, mFontSize, mSettings.label_style).width; }
    double max_label_offset();

    void make_coloring();

}; // class TreeDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
