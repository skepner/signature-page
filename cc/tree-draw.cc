#include <algorithm>

#include "tree-draw.hh"
#include "tree.hh"
#include "tree-iterate.hh"
#include "surface.hh"
#include "coloring.hh"

// ----------------------------------------------------------------------

TreeDraw::TreeDraw(Surface& aSurface, Tree& aTree, const TreeDrawSettings& aSettings, const HzSections& aHzSections)
    : mSurface(aSurface), mTree(aTree), mSettings(aSettings), mHzSections(aHzSections)
{
    make_coloring();
}

// ----------------------------------------------------------------------

// explicit destructor made here to allow proper mColoring destruction
TreeDraw::~TreeDraw()
{

} // TreeDraw::~TreeDraw

// ----------------------------------------------------------------------

void TreeDraw::make_coloring()
{
    if (mSettings.color_nodes == "black")
        mColoring = std::unique_ptr<Coloring>(new ColoringBlack());
    else if (mSettings.color_nodes == "continent")
        mColoring = std::unique_ptr<Coloring>(new ColoringByContinent());
    else {
        try {
            mColoring = std::unique_ptr<Coloring>(new ColoringByPos(std::stoul(mSettings.color_nodes)));
        }
        catch (std::exception&) {
            throw std::runtime_error("Unrecognized TreeDrawSettings.color_nodes: " + mSettings.color_nodes);
        }
    }

} // TreeDraw::make_coloring

// ----------------------------------------------------------------------

void TreeDraw::prepare()
{
    mTree.compute_cumulative_edge_length();
    hide_leaves();
    const size_t number_of_hz_sections = prepare_hz_sections();
    set_line_no();
    const auto canvas_size = mSurface.size();
    mHorizontalStep = canvas_size.width / mTree.width(mSettings.hide_if_cumulative_edge_length_bigger_than);
    mVerticalStep = canvas_size.height / (static_cast<double>(mTree.height() + 2) + (number_of_hz_sections - 1) * mHzSections.vertical_gap); // +2 to add space at the top and bottom
    set_vertical_pos();

} // TreeDraw::prepare

// ----------------------------------------------------------------------

void TreeDraw::draw()
{
    mLineWidth = mSettings.force_line_width ? mSettings.line_width : std::min(mSettings.line_width, mVerticalStep * 0.5);
    std::cout << "Tree line width: " << mLineWidth << "  Settings: " << mSettings.line_width << "  vertical_step/2: " << mVerticalStep * 0.5 << std::endl;
    fit_labels_into_viewport();

    double vertical_gap = 0;
    draw_node(mTree, mLineWidth / 2, vertical_gap, mSettings.root_edge);
    mColoring->report();
    draw_legend();

    // mark_nodes(aSurface);

} // TreeDraw::draw

// ----------------------------------------------------------------------

void TreeDraw::hide_leaves()
{
    auto hide_show_leaf = [this](Node& aNode) {
        aNode.draw.shown = ! (aNode.data.date() < mSettings.hide_isolated_before || aNode.data.cumulative_edge_length > mSettings.hide_if_cumulative_edge_length_bigger_than);
    };

    auto hide_show_branch = [](Node& aNode) {
        aNode.draw.shown = false;
        for (const auto& node: aNode.subtree) {
            if (node.draw.shown) {
                aNode.draw.shown = true;
                break;
            }
        }
    };

    tree::iterate_leaf_post(mTree, hide_show_leaf, hide_show_branch);

} // TreeDraw::hide_leaves

// ----------------------------------------------------------------------

void TreeDraw::set_line_no()
{
    size_t current_line = 1;    // line of the first node is 1, we have 1 line space at the top and bottom of the tree
    auto set_line_no = [&current_line](Node& aNode) {
        if (aNode.draw.shown) {
            aNode.draw.line_no = current_line;
            ++current_line;
        }
    };
    tree::iterate_leaf(mTree, set_line_no);
    std::cout << "Lines in the tree: " << current_line << std::endl;

} // TreeDraw::set_line_no

// ----------------------------------------------------------------------

void TreeDraw::set_vertical_pos()
{
    double vertical_pos = mVerticalStep;
    auto set_leaf_vertical_pos = [&](Node& aNode) {
        if (aNode.draw.shown) {
            if (aNode.draw.hz_section_start)
                vertical_pos += mHzSections.vertical_gap;
            aNode.draw.vertical_pos = vertical_pos;
            vertical_pos += mVerticalStep;
        }
    };
    tree::iterate_leaf(mTree, set_leaf_vertical_pos);

    auto set_intermediate_vertical_pos = [&](Node& aNode) {
        if (aNode.draw.shown) {
            double top = -1, bottom = -1;
            for (const auto& subnode: aNode.subtree) {
                if (subnode.draw.shown) {
                    bottom = subnode.draw.vertical_pos;
                    if (top < 0)
                        top = bottom;
                }
            }
            aNode.draw.vertical_pos = (top + bottom) / 2;
        }
    };
    tree::iterate_post(mTree, set_intermediate_vertical_pos);

} // TreeDraw::set_vertical_pos

// ----------------------------------------------------------------------

size_t TreeDraw::prepare_hz_sections()
{
    size_t number_of_hz_sections = 0;
    const Node& first_leaf = find_first_leaf(mTree);
    for (const auto& section: mHzSections.sections) {
        if (section.show) {
            Node* section_start = mTree.find_leaf_by_seqid(section.name);
            if (section_start) {
                if (section_start != &first_leaf) {
                    section_start->draw.hz_section_start = true;
                }
                ++number_of_hz_sections;
            }
            else {
                std::cerr << "WARNING: HzSection seq_id not found: " << section.name << std::endl;
            }
        }
    }
    if (number_of_hz_sections == 0)
        number_of_hz_sections = 1;
    return number_of_hz_sections;

} // TreeDraw::prepare_hz_sections

// ----------------------------------------------------------------------

void TreeDraw::calculate_name_offset()
{
    const auto tsize = mSurface.text_size("W", mFontSize, mSettings.label_style);
    mNameOffset  = mSettings.name_offset * tsize.width;
      // mLabelHeight = tsize.height;

} // TreeDraw::calculate_name_offset

// ----------------------------------------------------------------------

void TreeDraw::fit_labels_into_viewport()
{
    mFontSize = mVerticalStep;

    const double canvas_width = mSurface.size().width;

    for (double label_offset = max_label_offset(); label_offset > canvas_width; label_offset = max_label_offset()) {
        const double scale = std::min(canvas_width / label_offset, 0.99); // to avoid too much looping
          // std::cerr << "Canvas:" << canvas_width << " label_right:" << label_offset << " scale: " << scale << std::endl;
        mHorizontalStep *= scale;
        mFontSize *= scale;
    }

} // TreeDraw::fit_labels_into_viewport

// ----------------------------------------------------------------------

double TreeDraw::max_label_offset()
{
    calculate_name_offset();

    double max_label_origin = 0, max_label_right = 0;
    auto label_offset = [&](Node& node) {
        if (node.draw.shown) {
            const double label_origin = node.data.cumulative_edge_length * mHorizontalStep + mNameOffset;
            max_label_origin = std::max(max_label_origin, label_origin);
            max_label_right = std::max(max_label_right, label_origin + this->text_width(node.display_name()));
        }
    };
    tree::iterate_leaf(mTree, label_offset);
    return max_label_right; // std::make_pair(max_label_origin, max_label_right);

} // TreeDraw::max_label_offset

// ----------------------------------------------------------------------

void TreeDraw::draw_node(const Node& aNode, double aOriginX, double& aVerticalGap, double aEdgeLength)
{
    if (aNode.draw.shown) {
          // Location origin{aOriginX, aNode.draw.vertical_pos};
        Size size{(aEdgeLength < 0.0 ? aNode.edge_length : aEdgeLength) * mHorizontalStep, 0};
        const double right = aOriginX + size.width;

        if (aNode.is_leaf()) {
            const std::string text = aNode.display_name();
            const auto tsize = mSurface.text_size(text, mFontSize, mSettings.label_style);
            const Location text_origin{right + mNameOffset, aNode.draw.vertical_pos + tsize.height / 2};
            mSurface.text(text_origin, text, mColoring->color(aNode), mFontSize, mSettings.label_style);
            if (!aNode.draw.vaccine_label.empty()) {
                const auto& settings = mSettings.vaccine(aNode.draw.vaccine_label);
                Size label_offset{-20, 20}; // TODO: settings
                const Location label_origin = text_origin + label_offset;
                mSurface.text(label_origin, aNode.draw.vaccine_label, settings.label_color, settings.label_size, settings.label_style);
                const auto vlsize = mSurface.text_size(aNode.draw.vaccine_label, settings.label_size, settings.label_style);
                const auto line_origin = label_origin + Size(vlsize.width / 2, label_offset.height > 0 ? - vlsize.height : 0);
                mSurface.line(line_origin, text_origin, settings.line_color, settings.line_width);
            }
        }
        else {
            double top = -1, bottom = -1;
            for (auto& node: aNode.subtree) {
                if (node.draw.shown) {
                    draw_node(node, right, aVerticalGap);
                    if (top < 0)
                        top = node.draw.vertical_pos;
                    if (node.draw.vertical_pos > bottom)
                        bottom = node.draw.vertical_pos;
                }
            }
            mSurface.line({right, top}, {right, bottom}, mSettings.line_color, mLineWidth, Surface::LineCap::Square);
        }
        mSurface.line({aOriginX, aNode.draw.vertical_pos}, {right, aNode.draw.vertical_pos}, mSettings.line_color, mLineWidth);
        draw_aa_transition(aNode, {aOriginX, aNode.draw.vertical_pos}, right);
    }

} // TreeDraw::draw_node

// ----------------------------------------------------------------------

void TreeDraw::draw_aa_transition(const Node& aNode, const Location& aOrigin, double aRight)
{
    const auto& settings = mSettings.aa_transition;
    if (settings.show && !aNode.data.aa_transitions.empty() && aNode.data.number_strains >= settings.number_strains_threshold) {
        std::vector<std::pair<std::string, const Node*>> labels;
        aNode.data.aa_transitions.make_labels(labels, settings.show_empty_left);
        if (!labels.empty()) {
            const auto& branch_settings = settings.per_branch;
            const auto longest_label = std::max_element(labels.begin(), labels.end(), [](const auto& a, const auto& b) { return a.first.size() < b.first.size(); });
            const auto longest_label_size = mSurface.text_size(longest_label->first, branch_settings.size, branch_settings.style);
            const auto node_line_width = aRight - aOrigin.x;
            Size offset(node_line_width > longest_label_size.width ? (node_line_width - longest_label_size.width) / 2 : (node_line_width - longest_label_size.width),
                        longest_label_size.height * branch_settings.interline);
            offset += branch_settings.label_offset;
            Location origin = aOrigin + offset;
            for (const auto& label: labels) {
                const auto label_width = mSurface.text_size(label.first, branch_settings.size, branch_settings.style).width;
                const Location label_xy(origin.x + (longest_label_size.width - label_width) / 2, origin.y);
                mSurface.text(label_xy, label.first, branch_settings.color, branch_settings.size, branch_settings.style);
                if (settings.show_node_for_left_line && label.second) {
                    mSurface.line(Location{},
                                  Location(mHorizontalStep * label.second->data.cumulative_edge_length, mVerticalStep * label.second->draw.line_no),
                                  settings.node_for_left_line_color, settings.node_for_left_line_width);
                }
                origin.y += longest_label_size.height * branch_settings.interline;
            }

            const Location connection_line_start{(aOrigin.x + aRight) / 2, aOrigin.y};
            const Location connection_line_end{aOrigin.x + longest_label_size.width / 2 + offset.width, aOrigin.y - longest_label_size.height + offset.height};
            if (distance(connection_line_start, connection_line_end) > 10) {
                mSurface.line(connection_line_start, connection_line_end, branch_settings.label_connection_line_color, mLineWidth /*branch_settings.label_connection_line_width*/);
            }

            std::cout << "AA transitions: ";
            std::transform(labels.begin(), labels.end(), std::ostream_iterator<std::string>(std::cout, " "), [](const auto& e) -> std::string { return e.first; });
            std::cout << " --> " << /* aNode.branch_id << */ " number_strains: " << aNode.data.number_strains << std::endl;
        }
    }

} // TreeDraw::draw_aa_transition

// ----------------------------------------------------------------------

const Legend* TreeDraw::coloring_legend() const
{
    if (!mColoringLegend && mColoring)
        mColoringLegend = std::unique_ptr<Legend>(mColoring->legend());
    return mColoringLegend.get();

} // TreeDraw::coloring_legend

// ----------------------------------------------------------------------

void TreeDraw::draw_legend()
{
    const Legend* legend = coloring_legend();
    if (legend) {
        Surface& legend_surface{mSurface.subsurface(mSettings.legend.offset, {mSettings.legend.width,  mSettings.legend.width / legend->size().aspect()}, legend->size().width, false)};
        legend->draw(legend_surface, mSettings.legend);
          // legend_surface->border("red", 1);
    }

} // TreeDraw::draw_legend

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
