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

#ifndef EXAMPLEFASTIMAGE_FLOODSTRATEGY_H
#define EXAMPLEFASTIMAGE_FLOODSTRATEGY_H

#include <iostream>
#include <queue>
#include "Pixel.h"

template<typename UserType>
class FloodStrategy {
private:
  uint32_t
      _imageWidth,
      _imageHeight;

  uint64_t *
      _labels;

  static uint64_t
      _currentLabel;

  Pixel<UserType> *
      _pixels;

  std::queue<Pixel<UserType> *>
      _toVisit;
public:
  FloodStrategy(uint32_t imageWidth, uint32_t imageHeight) :
      _imageWidth(imageWidth), _imageHeight(imageHeight) {
    _pixels = new Pixel<UserType>[imageWidth * imageHeight];
    _labels = new uint64_t[imageWidth * imageHeight]();
  }

  virtual ~FloodStrategy() {
    delete[] _pixels;
    delete[] _labels;
  }

  static uint64_t getCurrentLabel() { return _currentLabel; }

  uint64_t *getLabels() const { return _labels; }

  void setLabel(uint32_t row, uint32_t col) {
    this->_labels[row * _imageWidth + col] = FloodStrategy::_currentLabel;
  }

  void insertPixel(uint32_t r, uint32_t c, UserType value) {
    _pixels[r * _imageWidth + c] = Pixel<UserType>(value, r, c);
  }

  void addQueue(Pixel<UserType> *px) {
    px->setVisited(true);
    this->_toVisit.push(px);
  }

  Pixel<UserType> *getNextQueue() {
    auto px = _toVisit.front();
    _toVisit.pop();
    return px;
  }

  void visit4(Pixel<UserType> *px) {
    uint32_t
        row = px->getRow(),
        col = px->getCol();

    this->setLabel(row, col);
    if (row >= 1) {
      if (_pixels[(row - 1) * _imageWidth + col].needVisit()) {
        addQueue(&_pixels[(row - 1) * _imageWidth + col]);
      }
    }
    if (row + 1 < _imageHeight) {
      if (_pixels[(row + 1) * _imageWidth + col].needVisit()) {
        addQueue(&_pixels[(row + 1) * _imageWidth + col]);
      }
    }

    if (col >= 1) {
      if (_pixels[row * _imageWidth + col - 1].needVisit()) {
        addQueue(&_pixels[row * _imageWidth + col - 1]);
      }
    }
    if (col + 1 < _imageWidth) {
      if (_pixels[row * _imageWidth + col + 1].needVisit()) {
        addQueue(&_pixels[row * _imageWidth + col + 1]);
      }
    }
  }

  void visit8(Pixel<UserType> *px) {
    uint32_t
        uRow = px->getRow(),
        uCol = px->getCol();

    auto
        minRow = std::max(0, (int32_t) uRow - 1),
        maxRow = std::min((int32_t) _imageHeight, (int32_t) uRow + 2),
        minCol = std::max(0, (int32_t) uCol - 1),
        maxCol = std::min((int32_t) _imageWidth, (int32_t) uCol + 2);

    Pixel<UserType> *neighbour;

    px->setVisited(true);
    this->setLabel(uRow, uCol);
    for (int32_t row = minRow; row < maxRow; ++row) {
      for (int32_t col = minCol; col < maxCol; ++col) {
        neighbour = &_pixels[row * _imageWidth + col];
        if (neighbour->needVisit()) {
          addQueue(neighbour);
        }
      }
    }
  }

  void label(int connectivity) {
    Pixel<UserType> *px;
    bool flagProcess;
    for (uint32_t row = 0; row < _imageHeight; ++row) {
      for (uint32_t col = 0; col < _imageWidth; ++col) {
        px = &(_pixels[row * _imageWidth + col]);
        if (px->needVisit()) {
          do {
            px->setVisited(true);

            if (connectivity == 4) {
              visit4(px);
            } else {
              visit8(px);
            }
            flagProcess = false;

            if (!_toVisit.empty()) {
              px = getNextQueue();
              flagProcess = true;
            }

          } while (flagProcess);
          FloodStrategy::_currentLabel += 1;
        }
      }
    }
  }

  friend std::ostream &operator<<(
      std::ostream &os, const FloodStrategy &strategy) {
    os << "ImageWidth: " << strategy._imageWidth << " ImageHeight: " <<
       strategy._imageHeight << std::endl;

    for (uint32_t row = 0; row < strategy._imageHeight; ++row) {
      for (uint32_t col = 0; col < strategy._imageWidth; ++col) {
        os
            << (int) (strategy._pixels[row * strategy._imageWidth
                + col].getValue())
            << " ";
      }
      os << std::endl;
    }
    os << std::endl;
    return os;
  }

  void printLabels() {
    for (uint32_t row = 0; row < _imageHeight; ++row) {
      for (uint32_t col = 0; col < _imageWidth; ++col) {
        std::cout << (int) (_labels[row * _imageWidth + col]) << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
};

template<typename UserType>
uint64_t FloodStrategy<UserType>::_currentLabel = 1;

#endif //EXAMPLEFASTIMAGE_FLOODSTRATEGY_H
