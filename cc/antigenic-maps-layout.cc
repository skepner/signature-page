#include "antigenic-maps-layout.hh"
#include "chart-draw.hh"
//#include "tree.hh"
#include "tree-draw.hh"
#include "tree-iterate.hh"
#include "signature-page.hh"

// ----------------------------------------------------------------------

AntigenicMapsLayoutDraw::~AntigenicMapsLayoutDraw()
{
} // AntigenicMapsLayoutDraw::~AntigenicMapsLayoutDraw

// ----------------------------------------------------------------------

void AntigenicMapsLayoutDraw::prepare()
{
    find_sequenced_antigens();
    prepare_apply_mods();
    const Viewport& viewport = mAntigenicMapsDraw.chart().viewport();
    settings().viewport(viewport);
    std::cout << "INFO: Using viewport: " << viewport << std::endl;

} // AntigenicMapsLayoutDraw::prepare

// ----------------------------------------------------------------------

const Viewport& AntigenicMapsLayoutDraw::viewport() const
{
    return mAntigenicMapsDraw.chart().viewport();
}

// ----------------------------------------------------------------------

void AntigenicMapsLayoutDraw::apply_mods_before(Surface& aSurface)
{
    for (const auto& mod: settings().mods) {
        const std::string name = mod.name();
        if (name == "background") {
            aSurface.background(mod.get_color("color", "black"));
        }
        else if (name == "grid") {
            aSurface.grid(Scaled{1}, mod.get_color("color", "grey80"), Pixels{mod.get_or_default("line_width", 1.0)});
        }
    }

} // AntigenicMapsLayoutDraw::apply_mods_before

// ----------------------------------------------------------------------

void AntigenicMapsLayoutDraw::apply_mods_after(Surface& aSurface)
{
    for (const auto& mod: settings().mods) {
        const std::string name = mod.name();
        if (name == "border") {
            aSurface.border(mod.get_color("color", "black"), Pixels{mod.get_or_default("line_width", 1.0)});
        }
    }

} // AntigenicMapsLayoutDraw::apply_mods_after

// ----------------------------------------------------------------------

void AntigenicMapsLayoutDraw::find_sequenced_antigens()
{
    size_t hz_section_index = NodeDrawData::HzSectionNoIndex;
    auto find_antigens = [this,&hz_section_index](const Node& aNode) {
        if (aNode.draw.hz_section_index != NodeDrawData::HzSectionNoIndex)
            hz_section_index = aNode.draw.hz_section_index;
        if (aNode.draw.chart_antigen_index != ChartDrawBase::AntigenNotFound)
            mSequencedAntigens[aNode.draw.chart_antigen_index] = hz_section_index;
    };

    tree::iterate_leaf(mAntigenicMapsDraw.tree(), find_antigens);

} // AntigenicMapsLayoutDraw::find_sequenced_antigens

// ----------------------------------------------------------------------

AntigenicMapsLayout::~AntigenicMapsLayout()
{
} // AntigenicMapsLayout::~AntigenicMapsLayout

// ----------------------------------------------------------------------

void LabelledGridBase::draw(Surface& aMappedAntigensDrawSurface, bool report_antigens_in_hz_sections)
{
    std::cerr << "INFO: AntigenicMapsLayoutDraw::draw" << std::endl;

    layout_draw().prepare_chart_for_all_sections();
    const AntigenicMapsDrawSettings& settings = layout_draw().settings();
    Surface& surface = layout_draw().surface();
    // std::cerr << "Maps " << surface << std::endl;

    const double map_width = (surface.viewport().size.width - (settings.columns - 1) * settings.gap) / settings.columns;

    size_t shown_maps = 0, row = 0, column = 0;
    for (const auto section_index: layout_draw().hz_sections().section_order) {
        const auto& section = layout_draw().hz_sections().sections[section_index];
        if (section.show && section.show_map) {
            Surface& map_surface = surface.subsurface({column * (map_width + settings.gap), row * (map_width + settings.gap)},
                                                      Scaled{map_width}, layout_draw().viewport(), true);
            std::cout << "===============================\nINFO: MAP " << section_index << ' ' << layout_draw().hz_sections().node_refs[section_index].index << ": " << map_surface << std::endl;
            // std::cerr << "Map " << map_surface << std::endl;
            // std::cerr << "origin_offset: " << map_surface.origin_offset() << "  scale: " << map_surface.scale() << std::endl;
            layout_draw().prepare_drawing_chart(section_index, report_antigens_in_hz_sections);
            layout_draw().draw_chart(map_surface, section_index);
            draw_mapped_antigens_section(section_index, aMappedAntigensDrawSurface);
            ++column;
            if (column >= settings.columns) {
                ++row;
                column = 0;
            }
            ++shown_maps;
        }
        else {
            std::cout << "===============================\nINFO: MAP " << section_index << ": not shown" << std::endl;
        }
    }

    const double antigenic_maps_width = layout_draw().signature_page_settings().antigenic_maps_width;
    const size_t rows = shown_maps / settings.columns + ((shown_maps % settings.columns) ? 1 : 0);
    const double maps_height = map_width * rows + (rows - 1) * settings.gap;
    const double suggested_surface_width = antigenic_maps_width * surface.viewport().size.height / maps_height;
    std::cout << "Map area height: " << maps_height << std::endl;
    if (std::abs((antigenic_maps_width - suggested_surface_width) / antigenic_maps_width) > 0.01) {
        std::cout << "Change signature_page.antigenic_maps_width from " << antigenic_maps_width << " to " << suggested_surface_width << '\n'
                  << "  If you use --init-settings changed width will be saved there" << '\n';
        layout_draw().signature_page_settings().antigenic_maps_width = suggested_surface_width;
    }
    else
        std::cout << "antigenic_maps.width is OK: " << antigenic_maps_width << " vs. suggested " << suggested_surface_width << '\n';

} // LabelledGridBase::draw

// ----------------------------------------------------------------------

void LabelledGridBase::draw_mapped_antigens_section(size_t aSectionIndex, Surface& aMappedAntigensDrawSurface)
{
    const AntigenicMapsDrawSettings& settings = layout_draw().settings();
    const auto& section = layout_draw().hz_sections().node_refs[aSectionIndex];

    const double left = 0, right = aMappedAntigensDrawSurface.viewport().size.width;
    const double top = section.first->draw.vertical_pos, bottom = section.last->draw.vertical_pos;
    aMappedAntigensDrawSurface.line({left, top}, {right, top}, settings.mapped_antigens_section_line_color, Pixels{settings.mapped_antigens_section_line_width}, Surface::LineCap::Square);
    aMappedAntigensDrawSurface.line({left, bottom}, {right, bottom}, settings.mapped_antigens_section_line_color, Pixels{settings.mapped_antigens_section_line_width}, Surface::LineCap::Square);
    aMappedAntigensDrawSurface.line({right, top}, {right, bottom}, settings.mapped_antigens_section_line_color, Pixels{settings.mapped_antigens_section_line_width}, Surface::LineCap::Square);

} // LabelledGridBase::draw_mapped_antigens_section

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
