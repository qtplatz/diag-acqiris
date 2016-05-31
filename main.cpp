/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include "acqiris.hpp"

int
main(int argc, char* argv[])
{
    namespace po = boost::program_options;
    
    po::variables_map vm;
    
    po::options_description desc("options");
    desc.add_options()
        ( "help,h", "print help message" )
        ( "delay",    po::value<double>()->default_value(0.0),  "start delay in microseconds" )
        ( "width",    po::value<double>()->default_value(10.0), "duration (width) in microseconds" )
        ( "mode",     po::value<int>()->default_value(0),       "config mode (0 = digitizer, 2 = averager)" )
        ( "channels", po::value<int>()->default_value(1),       "channels (1, 2 or 3 (both))" )
        ( "ch1.fs",     po::value<double>()->default_value(5.0),     "ch1 full scale (V)" )
        ( "ch1.offset", po::value<double>()->default_value(0.0),     "ch1 offset (V)" )
        ( "ch2.fs",     po::value<double>()->default_value(5.0),     "ch2 full scale (V)" )
        ( "ch2.offset", po::value<double>()->default_value(0.0),     "ch2 offset (V)" )
        ( "ext.fs",     po::value<double>()->default_value(5.0),     "ch2 full scale (V)" )
        ( "ext.offset", po::value<double>()->default_value(0.0),     "ch2 offset (V)" )
        ( "trig.slope", po::value<int>()->default_value(0),          "trigger slope (0 = positive, 1 = negative" )
        ( "trig.level", po::value<double>()->default_value(1000.0),  "trigger level(mV)" )            
        ;
    
    po::store( po::command_line_parser( argc, argv ).options( desc ).run(), vm );
    po::notify( vm );
    
    if ( vm.count( "help" ) ) {
        std::cout << desc;
        return 0;
    }

    // acqrscontrols::ap240::method m;
    // m.hor_.delay = vm["delay"].as<double>() * 1.0e-6;
    // m.hor_.width = vm["width"].as<double>() * 1.0e-6;
    // m.hor_.mode = vm["mode"].as<int>();
    // m.channels_ = vm["channels"].as<int>() & 03;
    // m.ch1_.fullScale = vm[ "ch1.fs" ].as<double>();
    // m.ch1_.offset    = vm[ "ch1.offset" ].as<double>();
    // m.ch2_.fullScale = vm[ "ch2.fs" ].as<double>();
    // m.ch2_.offset    = vm[ "ch2.offset" ].as<double>();
    // m.ext_.fullScale = vm[ "ext.fs" ].as<double>();
    // m.ext_.offset    = vm[ "ext.offset" ].as<double>();
    // m.trig_.trigSlope = vm[ "trig.slope" ].as<int>();
    // m.trig_.trigLevel1 = vm[ "trig.level" ].as<double>(); // mV | %fs
    // if ( m.hor_.mode == 2 )
    //     m.hor_.nbrAvgWaveforms = 8;

    bool simulation( false );

    if ( auto p = getenv( "AcqirisOption" ) ) {
        if ( strcmp( p, "simulate" ) == 0 )
            simulation = true;
    }
    
    acqiris aqrs;

    if ( aqrs.initialize() ) {
        if ( aqrs.findDevice( simulation ) ) {

            if ( aqrs.digitizer_setup( 0.0, 10.0e-6 ) ) {

                while ( aqrs.acquire() ) {
                    
                    auto rcode = aqrs.waitForEndOfAcquisition( 10000 );
                    if ( rcode == acqiris::success ) {
                        
                        AqDataDescriptor dataDesc;
                        AqSegmentDescriptor segDesc;
                        std::vector< int8_t > data;
                        
                        std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
                        if ( aqrs.readData( 1, dataDesc, segDesc, data ) ) {

                            double hi = double( uint64_t( segDesc.timeStampHi ) << 32 );
                            double lo = segDesc.timeStampLo;
                            double t = ( hi + lo ) * 1.0e-12;
                            std::cout << "acquire complete with success" << std::endl;                            
                            std::cout << "# Samples acquired: " << dataDesc.returnedSamplesPerSeg;
                            std::cout << " Time: "
                                      << boost::format( "%08x:%08x" ) % segDesc.timeStampHi % segDesc.timeStampLo
                                      << boost::format( "\t%lf(s)") % t
                                      << std::endl;
                            std::cout << "horPos: " << segDesc.horPos;

                        }
                        
                    } else if ( rcode == acqiris::error_timeout ) {
                        std::cout << "acquire complete with timeout" << std::endl;
                    } else {
                        std::cout << "acquire complete with error" << std::endl;
                        exit(0);
                    }
                }
                
            }
        }
    }

    return 0;

}
