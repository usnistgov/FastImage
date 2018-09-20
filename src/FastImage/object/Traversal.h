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
// Created by anb22 on 8/18/17.
//

/// @file Traversal.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  8/18/17
/// @brief Traversal pattern for an image

#ifndef FASTIMAGE_TRAVERSAL_H
#define FASTIMAGE_TRAVERSAL_H

#include <cstdint>
#include <vector>
#include <ostream>
#include <cmath>
#include <map>
#include <queue>
#include <iomanip>
#include "../data/DataType.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
  * @class Traversal Traversal.h <FastImage/object/Traversal.h>
  *
  * @brief Traversal object used to generate a vector of pair of coordinates.
  *
  * @details Different traversal patterns, to select all the tiles of an image.
  **/

class Traversal {
 public:
  /// \brief Construct a traversal, from it type and the image dimension.
  Traversal(TraversalType traversalType,
            uint32_t numTileRow,
            uint32_t numTileCol) : _traversalType(
      traversalType), _numTileRow(numTileRow), _numTileCol(numTileCol) {
    switch (_traversalType) {
      case TraversalType::SNAKE:
        _name = "Snake";
        snakeAlgorithm();
        break;
      case TraversalType::SPIRAL:
        _name = "Spiral";
        spiralAlgorithm();
        break;
      case TraversalType::NAIVE:
        _name = "Naive";
        naiveAlgorithm();
        break;
      case TraversalType::HILBERT:
        _name = "Hilbert";
        hilbertAlgorithm();
        break;
      case TraversalType::DIAGONAL:
        _name = "Diagonal";
        diagonalAlgorithm();
        break;
    }
  }

  /// \brief Get the traversal vector
  /// \return Traversal vector
  const std::vector<std::pair<uint32_t, uint32_t>> &getTraversal() const {
    return _traversal;
  }

  /// \brief Get the traversal as a queue
  /// \return Traversal queue
  std::queue<std::pair<uint32_t, uint32_t>> getQueue() {
    std::queue<std::pair<uint32_t, uint32_t>>
        fifo;

    for (auto step : _traversal) {
      fifo.push(step);
    }
    return fifo;
  };

  /// \brief Get the number of tiles in a row
  /// \return Number of tiles in a row
  uint32_t getNumTileRow() const {
    return _numTileRow;
  }

  /// \brief Get the number of tiles in a column
  /// \return Number of tiles in a column
  uint32_t getNumTileCol() const {
    return _numTileCol;
  }

  /// \brief Get traversal name
  /// \return Traversal name
  const std::string &getName() const {
    return _name;
  }

  /// \brief Output stream operator
  /// \param os output stream
  /// \param traversal Traversal to print
  /// \return Output Stream with the traversal printed
  friend std::ostream &operator<<(std::ostream &os,
                                  const Traversal &traversal) {
    auto
        numTileRow = traversal.getNumTileRow(),
        numTileCol = traversal.getNumTileCol();

    int
        sizeValue = 0;

    float
        value = numTileCol * numTileCol;

    if (value < 0)
      value = -value;
    while (value != (int) value)
      value *= 10;
    while ((int) value != 0) {
      value /= 10;
      sizeValue++;
    }

    std::vector<std::vector<uint32_t>>
        mapToPrint(numTileRow, std::vector<uint32_t>(numTileCol, 0));
    uint32_t stepNumber = 0;

    for (auto step: traversal.getTraversal()) {
      if (step.first < numTileRow && step.second < numTileCol)
        mapToPrint[step.first][step.second] = stepNumber;
      stepNumber++;
    }

    os << "Traversal " << traversal.getName() << " ("
       << traversal.getNumTileRow() << "x"
       << traversal.getNumTileCol() << ")" << std::endl;

    for (auto row: mapToPrint) {
      for (auto item: row) {
        os << std::setw(sizeValue) << item << " ";
      }
      os << std::endl;
    }
    os << std::endl;

    return os;
  }

 private:
  /// \brief Determine the next step for the spiral algorithm
  /// \param direction Actual direction
  /// \param row Actual row
  /// \param col Actual column
  /// \param rowNext Next row
  /// \param colNext Next column
  void nextStep(Direction direction,
                uint32_t row,
                uint32_t col,
                int32_t &rowNext,
                int32_t &colNext) {
    switch (direction) {
      case Direction::NORTH:rowNext = row - 1;
        colNext = col;
        break;
      case Direction::SOUTH:rowNext = row + 1;
        colNext = col;
        break;
      case Direction::EAST:rowNext = row;
        colNext = col + 1;
        break;
      case Direction::WEST:rowNext = row;
        colNext = col - 1;
        break;
    }
  }

  /// \brief Test if the tile (row, col) is free
  /// \param rowTest Tile row to test
  /// \param colTest Tile column to test
  /// \param numTileRow Number tiles in a row
  /// \param numTileCol Number tiles in a column
  /// \param hasNotBeenVisited matrix of tiles visited
  /// \return True if tile free, else Fasle
  bool
  isFree(const int32_t &rowTest,
         const int32_t &colTest,
         const uint32_t &numTileRow,
         const uint32_t &numTileCol,
         const std::vector<std::vector<bool>> &hasNotBeenVisited) {
    return
        !(rowTest == -1 || (uint32_t) rowTest == numTileRow || colTest == -1 ||
            (uint32_t) colTest == numTileCol) &&
            hasNotBeenVisited[rowTest][colTest];
  }

  /// \brief Function used in Hilbert _traversal
  /// \param n Square region size
  /// \param x Column
  /// \param y Row
  /// \param rx Column rotational
  /// \param ry Row rotational
  void rot(uint32_t n, uint32_t &x, uint32_t &y, uint32_t rx, uint32_t ry) {
    if (ry == 0) {
      if (rx == 1) {
        x = n - 1 - x;
        y = n - 1 - y;
      }
      std::swap(x, y);
    }
  }

  /// \brief Function used in the hilbert algorithm
  /// \param n Square region size
  /// \param d Distance from the origin
  /// \param x Column
  /// \param y Row
  void d2xy(uint32_t n, uint32_t d, uint32_t &x, uint32_t &y) {
    uint32_t rx, ry, s, t = d;
    x = y = 0;
    for (s = 1; s < n; s *= 2) {
      rx = (uint32_t) 1 & (t / (uint32_t) 2);
      ry = (uint32_t) 1 & (t ^ rx);
      rot(s, x, y, rx, ry);
      x += s * rx;
      y += s * ry;
      t /= 4;
    }
  }

  /// \brief Determine if a position is valid
  /// \param row row to test
  /// \param col column to test
  /// \param n Number of rows
  /// \param m Number of columns
  /// \return True if valid, else False
  bool isValid(int32_t &row, int32_t &col, uint32_t &n, uint32_t &m) {
    return !(row < 0 || row >= (int32_t) n || col >= (int32_t) m || col < 0);
  }

  /// \brief Generate a naive _traversal
  void naiveAlgorithm() {
    for (uint32_t row = 0; row < _numTileRow; ++row) {
      for (uint32_t col = 0; col < _numTileCol; ++col) {
        _traversal.emplace_back(row, col);
      }
    }
  }

  /// \brief Generate a snake _traversal
  void snakeAlgorithm() {
    for (uint32_t row = 0; row < _numTileRow; ++row) {
      if (row % 2 == 0) {
        for (uint32_t col = 0; col < _numTileCol; ++col) {
          _traversal.emplace_back(row, col);
        }
      } else {
        for (int32_t col = _numTileCol - 1; col > -1; --col) {
          _traversal.emplace_back(row, col);
        }
      }
    }
  }

  /// \brief Generate a diagonal _traversal
  void diagonalAlgorithm() {
    uint32_t
        pos = 0;
    int32_t
        i = 0,
        j = 0;
    _traversal.assign(_numTileRow * _numTileCol,
                      std::pair<uint32_t, uint32_t>(0, 0));

    for (int32_t k = 0; k < (int32_t) _numTileRow; k++) {
      _traversal[pos].first = (uint32_t) k;
      _traversal[pos].second = 0;
      pos++;
      i = k - 1;
      j = 1;

      while (isValid(i, j, _numTileRow, _numTileCol)) {
        _traversal[pos].first = (uint32_t) i;
        _traversal[pos].second = (uint32_t) j;
        pos++;
        i--;
        j++;
      }
    }

    for (int32_t k = 1; k < (int32_t) _numTileCol; k++) {
      _traversal[pos].first = (uint32_t) _numTileRow - 1;
      _traversal[pos].second = (uint32_t) k;
      pos++;
      i = _numTileRow - 2;
      j = k + 1;
      while (isValid(i, j, _numTileRow, _numTileCol)) {
        _traversal[pos].first = (uint32_t) i;
        _traversal[pos].second = (uint32_t) j;
        pos++;
        i--;
        j++;
      }
    }
  }

  /// \brief Generate a hilbert _traversal filled with a snake
  void hilbertAlgorithm() {
    uint32_t
        minX = 0,
        minY = 0,
        pos = 0,
        rowTemp = 0,
        colTemp = 0,
        minSize = (uint32_t) pow(2,
                                 (uint32_t) std::min(log2(_numTileRow),
                                                     log2(_numTileCol)));

    _traversal.resize(_numTileRow * _numTileCol,
                      std::pair<uint32_t, uint32_t>(0, 0));

    for (uint32_t d = 0; d < minSize * minSize; ++d) {
      d2xy(minSize, pos, rowTemp, colTemp);
      _traversal[pos].first = rowTemp;
      _traversal[pos].second = colTemp;
      if (_traversal[pos].first > minX) minX = _traversal[pos].first;
      if (_traversal[pos].second > minY) minY = _traversal[pos].second;
      pos += 1;
    }

    // Filling empty case
    for (uint32_t row = 0; row < _numTileRow; ++row) {
      if (row % 2 == 0) {
        for (uint32_t col = minY + 1; col < _numTileCol; ++col) {
          _traversal[pos].first = row;
          _traversal[pos].second = col;
          pos += 1;
        }
      } else {
        for (uint32_t col = _numTileCol - 1; col > minY; --col) {
          _traversal[pos].first = row;
          _traversal[pos].second = col;
          pos += 1;
        }
      }
    }
    for (uint32_t i = minY + 1; i < _numTileRow; ++i) {
      if (i % 2 == 0) {
        for (uint32_t j = 0; j < minX + 1; ++j) {
          _traversal[pos].first = i;
          _traversal[pos].second = j;
          pos += 1;
        }
      } else {
        for (int32_t j = minX; j >= 0; --j) {
          _traversal[pos].first = i;
          _traversal[pos].second = (uint32_t) j;
          pos += 1;
        }
      }
    }
  }

  /// \brief Generate a spiral _traversal
  void spiralAlgorithm() {
    static std::map<Direction, Direction> nextDirection = {
        {Direction::EAST, Direction::SOUTH},
        {Direction::SOUTH, Direction::WEST},
        {Direction::WEST, Direction::NORTH},
        {Direction::NORTH, Direction::EAST}};

    std::vector<std::vector<bool>>
        hasNotBeenVisited(_numTileRow, std::vector<bool>(_numTileCol, true));
    Direction
        direction = Direction::EAST;

    uint32_t
        row = 0,
        col = 0,
        step = 0;
    int32_t
        rowNext = 0,
        colNext = 0;

    _traversal.emplace_back((uint32_t) row, (uint32_t) col);
    hasNotBeenVisited[row][col] = false;

    while (step < _numTileRow * _numTileCol - 1) {
      nextStep(direction, row, col, rowNext, colNext);
      if (isFree(rowNext,
                 colNext,
                 _numTileRow,
                 _numTileCol,
                 hasNotBeenVisited)) {
        ++step;
        row = (uint32_t) rowNext;
        col = (uint32_t) colNext;
        hasNotBeenVisited[row][col] = false;
        _traversal.emplace_back(row, col);
      } else {
        direction = nextDirection[direction];
      }
    }
  }

  std::vector<std::pair<uint32_t, uint32_t >>
      _traversal;     ///< Traversal vector

  TraversalType
      _traversalType; ///< Traversal type

  uint32_t
      _numTileRow,    ///< Number of tiles in a row in the image
      _numTileCol;    ///< Number of tiles in a column in the image

  std::string
      _name;          ///< Name of the algorithm
};
}
#endif //FASTIMAGE_TRAVERSAL_H
