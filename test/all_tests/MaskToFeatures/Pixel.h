// NIST-developed software is provided by NIST as a public service. 
// You may use, copy and distribute copies of the  software in any  medium, 
// provided that you keep intact this entire notice. You may improve, 
// modify and create derivative works of the software or any portion of the 
// software, and you may copy and distribute such modifications or works. 
// Modified works should carry a notice stating that you changed the software 
// and should note the date and nature of any such change. Please explicitly 
// acknowledge the National Institute of Standards and Technology as the 
// source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY
// OF ANY KIND, EXPRESS, IMPLIED, IN FACT  OR ARISING BY OPERATION OF LAW, 
// INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST 
// NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION  OF THE SOFTWARE WILL 
// BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST 
// DOES NOT WARRANT  OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE 
// SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE 
// CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using 
// and distributing the software and you assume  all risks associated with 
// its use, including but not limited to the risks and costs of program 
// errors, compliance  with applicable laws, damage to or loss of data, 
// programs or equipment, and the unavailability or interruption of operation. 
// This software is not intended to be used in any situation where a failure 
// could cause risk of injury or damage to property. The software developed 
// by NIST employees is not subject to copyright protection within 
// the United States.

//
// Created by anb22 on 2/9/18.
//

#ifndef EXAMPLEFASTIMAGE_PIXEL_H
#define EXAMPLEFASTIMAGE_PIXEL_H

#include <cstdint>
#include <limits>

template<typename UserType>
class Pixel {
 private:
  uint32_t
      _row,
      _col;

  UserType
      _value;

  static UserType
      _minPixel;

  bool
      _visited;

 public:
  Pixel() : _row(0), _col(0), _value(0), _visited(false) {}

  Pixel(UserType value, uint32_t row, uint32_t col)
      : _row(row), _col(col), _value(value), _visited(false) {
    if (value < Pixel::_minPixel) {
      Pixel::_minPixel = value;
    }
  }

  uint32_t getRow() const { return _row; }

  uint32_t getCol() const { return _col; }

  UserType getValue() const { return _value; };

  bool needVisit() const {
    return this->_value != Pixel::getMinPixel() && !this->_visited;
  }

  void setVisited(bool visited) { _visited = visited; }

  static UserType getMinPixel();

};

template<typename UserType>
UserType Pixel<UserType>::_minPixel = std::numeric_limits<UserType>::max();

template<typename UserType>
UserType Pixel<UserType>::getMinPixel() {
  return Pixel::_minPixel;
}

#endif //EXAMPLEFASTIMAGE_PIXEL_H
