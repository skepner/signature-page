#include <iostream>
#include <string>
#include <memory>

#include "surface-cairo.hh"

// ----------------------------------------------------------------------

void draw(Surface& aSurface);

// ----------------------------------------------------------------------

int main(int /*argc*/, const char */*argv*/[])
{
    int exit_code = 0;
    try {
        PdfCairo surface("/tmp/tc.pdf", 500, 850);
        draw(surface);
        std::unique_ptr<Surface> sub{surface.clip({100, 500}, 0.3, 1.0)};
        draw(*sub);
    }
    catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

void draw(Surface& aSurface)
{
    aSurface.border("grey50", 20);
    aSurface.circle({500, 100}, 200, 1.0, 0.0, "black", 10);
    aSurface.circle({1000, 0}, 200, 1.0, 0.0, "black", 10);
    aSurface.circle({1000, 1700}, 200, 1.0, 0.0, "black", 10);
    aSurface.line({100, 100}, {300, 300}, "red", 10);
    aSurface.rectangle({100, 170}, {50, 70}, "orange", 5);
    aSurface.rectangle_filled({120, 200}, {250, 70}, "brown", 5, "#8080ffff");
    aSurface.circle({150, 170}, 100, 0.5, M_PI / 4.0, "violet", 5);
    aSurface.circle_filled({200, 220}, 100, 2, 0, 0x80FFA0FF, 5, 0x80A0FFA0);

          // cairo_set_font_size(mContext, 30);
          // cairo_show_text(mContext, "JOPA");
          // surface.text(Location(200, 100 + sz.height / 2), "Jopa", 0x00FF00, tsize, style);
}

// ----------------------------------------------------------------------
