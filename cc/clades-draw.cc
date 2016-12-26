#include <set>

#include "clades-draw.hh"
#include "tree-draw.hh"
#include "time-series-draw.hh"
#include "tree-iterate.hh"

// ----------------------------------------------------------------------

void CladeData::extend(const Node& node, size_t section_inclusion_tolerance)
{
    if (!sections.empty() && node.draw.line_no <= (sections.back().last->draw.line_no + section_inclusion_tolerance + 1)) {
        sections.back().last = &node;
    }
    else {
        sections.emplace_back(&node);
    }

} // CladeData::extend

// ----------------------------------------------------------------------

void CladeData::remove_small_sections(size_t section_exclusion_tolerance)
{
    auto exclude = [&](const auto& s) -> bool {
        return (s.last->draw.line_no - s.first->draw.line_no) < section_exclusion_tolerance;
    };
    sections.erase(std::remove_if(sections.begin(), sections.end(), exclude), sections.end());

} // CladeData::remove_small_sections

// ----------------------------------------------------------------------

std::ostream& operator << (std::ostream& out, const CladeSection& section)
{
    return out << section.first->seq_id << ':' << section.first->draw.line_no << ".." << section.last->seq_id << ':' << section.last->draw.line_no;

} // operator << CladeSection

// ----------------------------------------------------------------------

std::ostream& operator << (std::ostream& out, const CladeData& clade)
{
    for (const auto& section: clade.sections) {
        out << ' ' << section;
    }
    return out;

} // operator << CladeData

// ----------------------------------------------------------------------

void CladesDraw::prepare()
{
      // extract clades from aTree
    auto scan = [this](const Node& aNode) {
        if (aNode.draw.shown) {
            const auto* node_clades = aNode.data.clades();
            if (node_clades) {
                for (auto& c: *node_clades) {
                    auto p = mClades.emplace(c, aNode);
                    if (!p.second) { // the clade is already present, extend its range
                        p.first->second.extend(aNode, mSettings.for_clade(c).section_inclusion_tolerance);
                    }
                }
            }
        }
    };
    tree::iterate_leaf(mTree, scan);

      // remove small sections
    for (auto& clade: mClades)
        clade.second.remove_small_sections(mSettings.for_clade(clade.first).section_exclusion_tolerance);

    for (auto& c: mClades) {
        std::cerr << "Clade: " << c.first << c.second << std::endl;
    }

    assign_slots();

} // CladesDraw::prepare

// ----------------------------------------------------------------------

void CladesDraw::assign_slots()
{
    std::set<size_t> used_slots;
      // slots forced in settings
    for (auto& clade: mClades) {
        const size_t forced = mSettings.for_clade(clade.first).slot;
        if (forced != CladeDrawSettings::NoSlot) {
            clade.second.slot = forced;
            used_slots.insert(forced);
        }
    }

    size_t slot = 0;
    while (used_slots.count(slot))
        ++slot;

    for (auto& clade: mClades) {
        if (clade.second.slot == CladeDrawSettings::NoSlot && mSettings.for_clade(clade.first).show) {
            clade.second.slot = slot;
            used_slots.insert(slot);
            ++slot;
            while (used_slots.count(slot))
                ++slot;
        }
    }

} // CladesDraw::assign_slots

// ----------------------------------------------------------------------

void CladesDraw::init_settings()
{
    for (auto& clade: mClades) {
        auto p = std::find_if(mSettings.clades.begin(), mSettings.clades.end(), [&](const auto& c) { return c.name == clade.first; });
        if (p == mSettings.clades.end()) {
            mSettings.clades.emplace_back(clade.first);
            mSettings.clades.back().slot = clade.second.slot;
        }
        else {
            p->slot = clade.second.slot;
        }
    }

} // CladesDraw::init_settings

// ----------------------------------------------------------------------

void CladesDraw::draw()
{
      // mSurface.border("violet", 10);

    auto draw_lines = mTimeSeriesDraw.offset().width < mSurface.offset().width ? &CladesDraw::draw_right : &CladesDraw::draw_left;

    for (const auto& name_clade: mClades) {
        const auto& clade = name_clade.second;
        if (clade.slot != CladeDrawSettings::NoSlot) {
            const auto& for_clade = mSettings.for_clade(name_clade.first);
            for (const auto& section: clade.sections) {
                const double top = section.first->draw.vertical_pos - mTreeDraw.vertical_step() / 2;
                const double bottom = section.last->draw.vertical_pos + mTreeDraw.vertical_step() / 2;
                const double label_height = mSurface.text_size("W", for_clade.label_size, for_clade.label_style).height;
                double label_vpos = top + label_height;
                if (for_clade.label_position == "middle") {
                    label_vpos = (top + bottom + label_height) / 2.0;
                }
                else if (for_clade.label_position == "bottom") {
                    label_vpos = bottom;
                }
                (this->*draw_lines)(clade.slot, name_clade.first, top, bottom, label_vpos, for_clade);
            }
        }
    }

} // CladesDraw::draw

// ----------------------------------------------------------------------

void CladesDraw::draw_right(size_t aSlot, std::string aCladeName, double top, double bottom, double label_vpos, const CladeDrawSettings& for_clade)
{
    const auto x = (aSlot + 1) * mSettings.slot_width;
    mSurface.double_arrow({x, top}, {x, bottom}, for_clade.arrow_color, for_clade.line_width, for_clade.arrow_width);
    std::string name = for_clade.display_name.empty() ? aCladeName : for_clade.display_name;
    mSurface.text(Location{x, label_vpos} + for_clade.label_offset, name, for_clade.label_color, for_clade.label_size, for_clade.label_style, for_clade.label_rotation);
    const double ts_width = mTimeSeriesDraw.size().width;
    mSurface.line({x, top}, {-ts_width, top}, for_clade.separator_color, for_clade.separator_width);
    mSurface.line({x, bottom}, {-ts_width, bottom}, for_clade.separator_color, for_clade.separator_width);

} // CladesDraw::draw_right

// ----------------------------------------------------------------------

void CladesDraw::draw_left(size_t aSlot, std::string aCladeName, double top, double bottom, double label_vpos, const CladeDrawSettings& for_clade)
{
    const auto x = mSurface.size().width - (aSlot + 1) * mSettings.slot_width;
    mSurface.double_arrow({x, top}, {x, bottom}, for_clade.arrow_color, for_clade.line_width, for_clade.arrow_width);
    std::string name = for_clade.display_name.empty() ? aCladeName : for_clade.display_name;
    const double label_width = mSurface.text_size(name, for_clade.label_size, for_clade.label_style).width;
    mSurface.text(Location{x, label_vpos} + Size{- for_clade.label_offset.width - label_width, for_clade.label_offset.height},
                  name, for_clade.label_color, for_clade.label_size, for_clade.label_style, for_clade.label_rotation);
    const double ts_width = mTimeSeriesDraw.size().width;
    mSurface.line({x, top}, {-ts_width, top}, for_clade.separator_color, for_clade.separator_width);
    mSurface.line({x, bottom}, {-ts_width, bottom}, for_clade.separator_color, for_clade.separator_width);

} // CladesDraw::draw_left

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: