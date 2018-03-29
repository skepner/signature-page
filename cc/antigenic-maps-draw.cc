#include <typeinfo>

#include "acmacs-chart-2/factory-import.hh"
#include "tree-draw.hh"
#include "signature-page.hh"
#include "antigenic-maps-layout.hh"
#include "ace-antigenic-maps-draw.hh"

// ----------------------------------------------------------------------

AntigenicMapsDrawBase::AntigenicMapsDrawBase(acmacs::surface::Surface& aSurface, Tree& aTree, HzSections& aHzSections, SignaturePageDrawSettings& aSignaturePageDrawSettings, AntigenicMapsDrawSettings& aSettings)
    : mSurface(aSurface), mTree(aTree), mHzSections(aHzSections),
      mSignaturePageDrawSettings(aSignaturePageDrawSettings), mSettings(aSettings)
{

} // AntigenicMapsDrawBase::AntigenicMapsDrawBase

// ----------------------------------------------------------------------

AntigenicMapsDrawBase::~AntigenicMapsDrawBase()
{
}

// ----------------------------------------------------------------------

void AntigenicMapsDrawBase::layout(AntigenicMapsLayout* aLayout)
{
    mLayout.reset(aLayout);

} // AntigenicMapsDrawBase::layout

// ----------------------------------------------------------------------

void AntigenicMapsDrawBase::init_settings()
{
    // std::cerr << "DEBUG: AntigenicMapsDrawBase::init_settings" << std::endl;
    const size_t number_sections = hz_sections().shown_maps();
    switch (number_sections) {
      case 0:
      case 1:
      case 2:
      case 3:
          settings().columns = 1;
          break;
      case 4:
      case 5:
      case 6:
          settings().columns = 2;
          break;
      case 7:
      case 8:
      case 9:
          settings().columns = 3;
          break;
      default:
          settings().columns = 3;
          break;
    }
    size_t maps_per_column = number_sections / settings().columns + ((number_sections % settings().columns) == 0 ? 0 : 1);
    const double map_width = 150; // height is not available at this moment mSurface.viewport().size.height / (maps_per_column + settings().gap * (maps_per_column - 1));
    signature_page_settings().antigenic_maps_width = map_width * settings().columns + (settings().columns - 1) * settings().gap;

    std::cerr << "INFO: antigenic maps: columns:" << settings().columns << " maps_per_column:" << maps_per_column << " map_width:" << map_width << " width:" << signature_page_settings().antigenic_maps_width << std::endl;

    chart().init_settings();

} // AntigenicMapsDrawBase::init_settings

// ----------------------------------------------------------------------

void AntigenicMapsDrawBase::draw(acmacs::surface::Surface& aMappedAntigensDrawSurface, bool report_antigens_in_hz_sections)
{
    layout().draw(aMappedAntigensDrawSurface, report_antigens_in_hz_sections);

} // AntigenicMapsDrawBase::draw

// ----------------------------------------------------------------------

void AntigenicMapsDrawBase::prepare()
{
      // std::cerr << "DEBUG: AntigenicMapsDrawBase::prepare" << std::endl;
    if (static_cast<std::string>(settings().layout) == "labelled_grid") {
        make_layout();
    }
    else {
        throw std::runtime_error("Unrecognized antigenic maps layout: " + static_cast<std::string>(settings().layout));
    }
    layout().prepare();

} // AntigenicMapsDrawBase::prepare

// ----------------------------------------------------------------------

AntigenicMapsDrawBase* make_antigenic_maps_draw(std::string aChartFilename, acmacs::surface::Surface& aSurface, Tree& aTree, HzSections& aHzSections, SignaturePageDrawSettings& aSignaturePageDrawSettings, AntigenicMapsDrawSettings& aSettings)
{
    std::string error;
    try {
        return new AntigenicMapsDraw(aSurface, aTree, std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(aChartFilename, acmacs::chart::Verify::None, report_time::No)), aHzSections, aSignaturePageDrawSettings, aSettings);
    }
    catch (acmacs::chart::import_error& err) {
        error = err.what();
    }
    // try {
    //     return new sdb::AntigenicMapsDraw(aSurface, aTree, sdb::read_chart_from_sdb(aChartFilename), aHzSections, aSignaturePageDrawSettings, aSettings);
    // }
    // catch (ChartReadError& err) {
    //     error += std::string("\n[Not SDB] ") + err.what();
    // }
    throw ChartReadError(error);

} // make_antigenic_maps_draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
