#include "surface-cairo.hh"

// ----------------------------------------------------------------------

SurfaceCairo::~SurfaceCairo()
{
    if (mContext)
        cairo_destroy(mContext);

} // SurfaceCairo::~SurfaceCairo

// ----------------------------------------------------------------------

SurfaceCairo* SurfaceCairo::clip(const Location& aOffset, double aScale, double aAspect)
{
    return new SurfaceCairo(mContext, mOffset + aOffset, mScale * aScale, mAspect * aAspect);

} // SurfaceCairo::clip

// ----------------------------------------------------------------------

void SurfaceCairo::line(const Location& a, const Location& b, Color aColor, double aWidth, LineCap aLineCap)
{
    context(*this)
            .set_line_width(aWidth)
            .set_source_rgba(aColor)
            .set_line_cap(aLineCap)
            .move_to(a)
            .line_to(b)
            .stroke();

} // SurfaceCairo::line

// ----------------------------------------------------------------------

void SurfaceCairo::rectangle(const Location& a, const Size& s, Color aColor, double aWidth, LineCap aLineCap)
{
    context(*this)
            .set_line_width(aWidth)
            .set_line_cap(aLineCap)
            .rectangle(a, s)
            .set_source_rgba(aColor)
            .stroke();

} // SurfaceCairo::rectangle

// ----------------------------------------------------------------------

void SurfaceCairo::rectangle_filled(const Location& a, const Size& s, Color aOutlineColor, double aWidth, Color aFillColor, LineCap aLineCap)
{
    context(*this)
            .set_line_width(aWidth)
            .set_line_cap(aLineCap)
            .rectangle(a, s)
            .set_source_rgba(aFillColor)
            .fill_preserve()
            .set_source_rgba(aOutlineColor)
            .stroke();

} // SurfaceCairo::rectangle_filled

// ----------------------------------------------------------------------

void SurfaceCairo::circle(const Location& aCenter, double aDiameter, double aAspect, double aAngle, Color aOutlineColor, double aOutlineWidth)
{
    context(*this)
            .set_line_width(aOutlineWidth)
            .translate(aCenter)
            .rotate(aAngle)
            .scale(aAspect)
            .circle(aDiameter / 2)
            .set_source_rgba(aOutlineColor)
            .stroke();

} // SurfaceCairo::circle

// ----------------------------------------------------------------------

void SurfaceCairo::circle_filled(const Location& aCenter, double aDiameter, double aAspect, double aAngle, Color aOutlineColor, double aOutlineWidth, Color aFillColor)
{
    context(*this)
            .set_line_width(aOutlineWidth)
            .translate(aCenter)
            .rotate(aAngle)
            .scale(aAspect)
            .circle(aDiameter / 2)
            .set_source_rgba(aFillColor)
            .fill_preserve()
            .set_source_rgba(aOutlineColor)
            .stroke();

} // SurfaceCairo::circle_filled

// ----------------------------------------------------------------------

void SurfaceCairo::square_filled(const Location& aCenter, double aSide, double aAspect, double aAngle, Color aOutlineColor, double aOutlineWidth, Color aFillColor, LineCap aLineCap)
{
    context(*this)
            .set_line_width(aOutlineWidth)
            .set_line_cap(aLineCap)
            .translate(aCenter)
            .rotate(aAngle)
            .rectangle({- aSide / 2 * aAspect, - aSide / 2}, {aSide * aAspect, aSide})
            .set_source_rgba(aFillColor)
            .fill_preserve()
            .set_source_rgba(aOutlineColor)
            .stroke();

} // SurfaceCairo::square_filled

// ----------------------------------------------------------------------

void SurfaceCairo::triangle_filled(const Location& aCenter, double aSide, double aAspect, double aAngle, Color aOutlineColor, double aOutlineWidth, Color aFillColor, LineCap aLineCap)
{
    const auto cos_pi_6 = std::cos(M_PI / 6.0);
    const auto radius = aSide * cos_pi_6;
    context(*this)
            .set_line_width(aOutlineWidth)
            .set_line_cap(aLineCap)
            .translate(aCenter)
            .rotate(aAngle)
            .move_to({0, - radius})
            .line_to({- radius * cos_pi_6 * aAspect, radius * 0.5})
            .line_to({radius * cos_pi_6 * aAspect, radius * 0.5})
            .close_path()
            .set_source_rgba(aFillColor)
            .fill_preserve()
            .set_source_rgba(aOutlineColor)
            .stroke();

} // SurfaceCairo::triangle_filled

// ----------------------------------------------------------------------

void SurfaceCairo::path_outline(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last, Color aOutlineColor, double aOutlineWidth, bool aClose, LineCap aLineCap)
{
    context(*this)
            .new_path()
            .set_line_cap(aLineCap)
            .set_line_join(LineJoin::Miter)
            .set_line_width(aOutlineWidth)
            .set_source_rgba(aOutlineColor)
            .move_to(*first)
            .lines_to(first + 1, last)
            .close_path_if(aClose)
            .stroke();

} // SurfaceCairo::path_outline

// ----------------------------------------------------------------------

void SurfaceCairo::path_fill(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last, Color aFillColor)
{

} // SurfaceCairo::path_fill

// ----------------------------------------------------------------------

void SurfaceCairo::double_arrow(const Location& a, const Location& b, Color aColor, double aLineWidth, double aArrowWidth)
{

} // SurfaceCairo::double_arrow

// ----------------------------------------------------------------------

void SurfaceCairo::grid(double aStep, Color aLineColor, double aLineWidth)
{

} // SurfaceCairo::grid

// ----------------------------------------------------------------------

void SurfaceCairo::border(Color aLineColor, double aLineWidth)
{
    context(*this)
            .set_line_width(aLineWidth)
            .set_line_cap(LineCap::Butt)
            .rectangle({0.0, 0.0}, {canvas_width, canvas_width * mAspect})
            .set_source_rgba(aLineColor)
            .stroke();

} // SurfaceCairo::border

// ----------------------------------------------------------------------

void SurfaceCairo::text(const Location& a, std::string aText, Color aColor, double aSize, const TextStyle& aTextStyle, double aRotation)
{

} // SurfaceCairo::text

// ----------------------------------------------------------------------

Size SurfaceCairo::text_size(std::string aText, double aSize, const TextStyle& aTextStyle, double* x_bearing)
{

} // SurfaceCairo::text_size

// ----------------------------------------------------------------------

SurfaceCairo::context::context(SurfaceCairo& aSurface)
    : mContext(cairo_reference(aSurface.mContext))
{
    cairo_save(mContext);
    translate(aSurface.mOffset);
    scale(aSurface.mScale, aSurface.mScale);
    new_path();
    move_to();
    line_to({canvas_width, 0.0});
    line_to({canvas_width, canvas_width * aSurface.mAspect});
    line_to({0, canvas_width * aSurface.mAspect});
    close_path();
    clip();

} // SurfaceCairo::context::context

// ----------------------------------------------------------------------

PdfCairo::PdfCairo(std::string aFilename, double aWidth, double aHeight)
{
    auto surface = cairo_pdf_surface_create(aFilename.c_str(), aWidth, aHeight);
    mContext = cairo_create(surface);
    cairo_surface_destroy(surface);
    mScale = aWidth / canvas_width;
    mAspect = aHeight / aWidth;

} // PdfCairo::PdfCairo

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
