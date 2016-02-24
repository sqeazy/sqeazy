#ifndef _VIDEO_IO_H_
#define _VIDEO_IO_H_

#include <cstdint>
#include <fstream>

namespace sqeazy {

static uint32_t write_encoded(const std::string& _filename, std::vector<uint8_t>& _video ){
  
    std::ofstream ofile(_filename, std::ios::binary | std::ios::out );
  
    for ( const uint8_t& c : _video )
      ofile << c;

    ofile.close();

    return 0;
  }

};
#endif /* _VIDEO_IO_H_ */
