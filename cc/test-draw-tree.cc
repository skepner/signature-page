#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>

#pragma GCC diagnostic push
#include "acmacs-base/boost-diagnostics.hh"
#include "boost/program_options.hpp"
#pragma GCC diagnostic pop

#include "locationdb/locdb.hh"
#include "seqdb/seqdb.hh"
#include "surface-cairo.hh"
#include "tree.hh"
#include "tree-export.hh"
#include "tree-draw.hh"
#include "settings.hh"

// ----------------------------------------------------------------------

int get_args(int argc, const char *argv[], std::string& aSettingsFilename, std::string& aTreeFilename, std::string& aOutputPdf, std::string& aSeqdbFilename);
void read(Tree& tree, LocDb& locdb, seqdb::Seqdb& seqdb, std::string aTreeFilename, std::string aSeqdbFilename);
void draw(const Settings& aSettings, Surface& aSurface, Tree& tree);

// ----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
    std::string settings_filename, tree_filename, output_filename, seqdb_filename;
    int exit_code = get_args(argc, argv, settings_filename, tree_filename, output_filename, seqdb_filename);
    if (exit_code == 0) {
        try {
            LocDb locdb;
            seqdb::Seqdb seqdb;
            Tree tree;
            Settings settings;
            if (!settings_filename.empty())
                read_settings(settings, settings_filename);
            read(tree, locdb, seqdb, tree_filename, seqdb_filename);
            PdfCairo surface(output_filename, 500, 850);
            surface.background("white");
            const double offset = 100;
            // const double scale = (surface.width() - offset * 4) / surface.width();
            // std::cout << "Sub scale:" << scale << std::endl;
            std::unique_ptr<Surface> sub{surface.subsurface({offset, offset}, surface.size() - Size(offset * 2, offset * 2), surface.size().width, false)};
            sub->border(0xA0FFA000, 10);
            draw(settings, *sub, tree);
        }
        catch (std::exception& err) {
            std::cerr << err.what() << std::endl;
            exit_code = 1;
        }
    }
    return exit_code;
}

// ----------------------------------------------------------------------

int get_args(int argc, const char *argv[], std::string& aSettingsFilename, std::string& aTreeFilename, std::string& aOutputPdf, std::string& aSeqdbFilename)
{
    using namespace boost::program_options;
    options_description desc("Options");
    desc.add_options()
            ("help", "Print help messages")
            ("seqdb", value<std::string>(&aSeqdbFilename)/* ->required() */, "path to seqdb")
            ("settings,s", value<std::string>(&aSettingsFilename), "signature page drawing settings (json) filename")
            ("tree", value<std::string>(&aTreeFilename)->required(), "path to tree to draw")
            ("output,o", value<std::string>(&aOutputPdf)->required(), "output pdf")
            ;
    positional_options_description pos_opt;
    pos_opt.add("tree", 1);
    pos_opt.add("output", 1);

    variables_map vm;
    try {
        store(command_line_parser(argc, argv).options(desc).positional(pos_opt).run(), vm);
        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            return 1;
        }
        notify(vm);
        return 0;
    }
    catch(required_option& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
          // std::cerr << "Usage: " << argv[0] << " <tree.json> <output.pdf>" << std::endl;
        return 2;
    }
    catch(error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return 3;
    }

} // get_args

// ----------------------------------------------------------------------

void read(Tree& tree, LocDb& locdb, seqdb::Seqdb& seqdb, std::string aTreeFilename, std::string aSeqdbFilename)
{
    tree_import(aTreeFilename, tree);
    locdb.importFrom(std::getenv("ACMACSD_ROOT") + std::string("/data/locationdb.json.xz"));
    tree.set_continents(locdb);
    if (!aSeqdbFilename.empty()) {
        seqdb.load(aSeqdbFilename);
        tree.match_seqdb(seqdb);
    }
      // tree.report_cumulative_edge_length(std::cout);
    tree.ladderize(Tree::LadderizeMethod::NumberOfLeaves);           // must be before clade_setup
    tree.make_aa_transitions();
    tree.add_vaccine("A/SOUTH%20AUCKLAND/17/2016__MDCK%3F/SIAT1", "A/SOUTH AUCKLAND/17/2016 C");

} // read

// ----------------------------------------------------------------------

void draw(const Settings& aSettings, Surface& aSurface, Tree& tree)
{
    aSurface.background("white");
    TreeDraw tree_draw{aSurface, tree, aSettings.tree_draw};
    tree_draw.prepare();
    tree_draw.draw();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
