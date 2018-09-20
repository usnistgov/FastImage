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

/// @file Blob.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  4/6/18
/// @brief Blob object representing a part of a feature


#ifndef FEATURECOLLECTION_BLOB_H
#define FEATURECOLLECTION_BLOB_H

#include <cstdint>
#include <utility>
#include <set>
#include <map>
#include <mutex>
#include <list>
#include <iostream>
#include <FastImage/api/FastImage.h>
#include <FastImage/TileLoadersExamples/TiffTileLoader.h>
#include <unordered_set>

/// \namespace fc FeatureCollection namespace
namespace fc {

/// \brief Coordinate structure as a pair of int32_t for <row, col>
using Coordinate = std::pair<int32_t, int32_t>;


/**
  * @class Blob Blob.h <FastImage/FeatureCollection/Data/Blob.h>
  *
  * @brief Blob representing a part of a feature
  *
  * @details It is composed by a bounding box from the point (rowMin, rowMax) to
  * (rowMAx, ColMax).The bounding box delimit a sparse matrix representing the
  * pixels part of this blob. A blob has a specific id, called tag.
  **/
class Blob {
 public:
  /// \brief Blob construction and initialisation
  Blob()
      : _parent(this),
        _rank(0),
        _rowMin(std::numeric_limits<int32_t>::max()),
        _rowMax(0),
        _colMin(std::numeric_limits<int32_t>::max()),
        _colMax(0) {
    static uint32_t
        currentTag = 0;
    std::mutex
        m;
    std::lock_guard<std::mutex>
        lock(m);

    _tag = currentTag++;
    _count = 0;
  }

  /// \brief Get Blob tag
  /// \return Blob tag
  uint32_t getTag() const { return _tag; }

  /// \brief Get sparse matrix structure
  /// \return Sparse matrix structure
  std::unordered_map<int32_t, std::unordered_set<int32_t>>
  &getRowCols() {
    return _rowCols;
  }
  /// \brief Get minimum bounding box row
  /// \return Minimum bounding box row
  int32_t getRowMin() const { return _rowMin; }

  /// \brief Get maximum bounding box row
  /// \return Maximum bounding box row
  int32_t getRowMax() const { return _rowMax; }

  /// \brief Get minimum bounding box col
  /// \return Minimum bounding box col
  int32_t getColMin() const { return _colMin; }

  /// \brief Get maximum bounding box col
  /// \return Maximum bounding box col
  int32_t getColMax() const { return _colMax; }

  /// \brief Get number of pixels in a blob
  /// \return Number of pixels in a blob
  uint64_t getCount() const { return _count; }

  /// \brief Get blob parent, used by Union find
  /// \return Blob  parent
  Blob *getParent() const { return _parent; }

  /// \brief Get blob rank, used by Union find
  /// \return Blob rank
  uint32_t getRank() const { return _rank; }

  /// \brief Test if a pixel is in a blob
  /// \param row Row asked
  /// \param col Col asked
  /// \return True is the pixel(row, col) is in the blob, else False
  bool isPixelinFeature(int32_t row, int32_t col) {
    if (row >= _rowMin && row < _rowMax && col >= _colMin && col < _colMax) {
      if (_rowCols.count(row) != 0) {
        if (_rowCols[row].count(col) != 0) {
          return true;
        }
      }
    }
    return false;
  }

  /// \brief Minimum bounding box row setter
  /// \param rowMin Minimum bounding box row
  void setRowMin(int32_t rowMin) { _rowMin = rowMin; }

  /// \brief Maximum bounding box row setter
  /// \param rowMax Maximum bounding box row
  void setRowMax(int32_t rowMax) { _rowMax = rowMax; }

  /// \brief Minimum bounding box col setter
  /// \param colMin Minimum bounding box col
  void setColMin(int32_t colMin) { _colMin = colMin; }

  /// \brief Maximum bounding box col setter
  /// \param colMax Maximum bounding box col
  void setColMax(int32_t colMax) { _colMax = colMax; }

  /// \brief Count setter
  /// \param count Pixel Count
  void setCount(uint64_t count) { _count = count; }

  /// \brief Blob parent setter Used by Union find algorithm
  /// \param parent Blob parent
  void setParent(Blob *parent) { _parent = parent; }

  /// \brief Blob rank setter Used by Union find algorithm
  /// \param rank Blob rank
  void setRank(uint32_t rank) { _rank = rank; }

  /// \brief Add a pixel to the blob and update blob metadata
  /// \param row Pixel's row
  /// \param col Pixel's col
  void addPixel(int32_t row, int32_t col) {
    if (row < _rowMin)
      _rowMin = row;
    if (col < _colMin)
      _colMin = col;
    if (row >= _rowMax)
      _rowMax = row + 1;
    if (col >= _colMax)
      _colMax = col + 1;
    _count++;
    addRowCol(row, col);
  }

  /// \brief Merge 2 blobs, and delete the unused one
  /// \param blob Blob to merge with the current
  /// \return The blob merged
  Blob *mergeAndDelete(Blob *blob) {
    Blob
        *toDelete = nullptr,
        *destination = nullptr;

    // Decide which one (between this and blob) will be deleted
    if (this->getCount() >= blob->getCount()) {
      destination = this;
      toDelete = blob;
    } else {
      destination = blob;
      toDelete = this;
    }

    // Set blob metadata
    destination->setRowMin(
        std::min(destination->getRowMin(), toDelete->getRowMin()));
    destination->setColMin(
        std::min(destination->getColMin(), toDelete->getColMin()));
    destination->setRowMax(
        std::max(destination->getRowMax(), toDelete->getRowMax()));
    destination->setColMax(
        std::max(destination->getColMax(), toDelete->getColMax()));

    // Merge sparse matrix
    for (auto rowCol : toDelete->getRowCols()) {
      for (auto col : rowCol.second) {
        destination->addRowCol(rowCol.first, col);
      }
    }

    // update pixel count
    destination->setCount(toDelete->getCount() + destination->getCount());

    // Delete unused Blob
    delete (toDelete);
    // Return merged blob
    return destination;
  }

  /// \brief Print blob state
  /// \param os Output stream
  /// \param blob Blob to print
  /// \return Output stream with the blob information
  friend std::ostream &operator<<(std::ostream &os, const Blob &blob) {
    os << "Blob #" << blob._tag << std::endl;
    os << "    BB: ["
       << blob._rowMin << ", " << blob._colMin << "] -> ["
       << blob._rowMax << ", " << blob._colMax << "]" << std::endl;
    os << "    # Pixels: " << blob._count << std::endl;
    return os;
  }

 private:
  /// \brief Add a pixel to the sparse matrix
  /// \param row Pixel row
  /// \param col Pixel col
  void addRowCol(int32_t row, int32_t col) { _rowCols[row].insert(col); }

  Blob *
      _parent = nullptr;  ///< Blob parent, used by the Union find algorithm

  uint32_t
      _rank = 0,          ///< Blob rank, used by the Union find algorithm
      _tag = 0;           ///< Tag, only used to print/debug purpose

  int32_t
      _rowMin{},          ///< Minimum bounding box row
      _rowMax{},          ///< Maximum bounding box row
      _colMin{},          ///< Minimum bounding box col
      _colMax{};          ///< Maximum bounding box col

  uint64_t
      _count{};           ///< Number of pixel to fastened the blob merge

  std::unordered_map<int32_t, std::unordered_set<int32_t>>
      _rowCols
      {};         ///< Sparse matrix of unique coordinates composing the blob
};

}

#endif //FEATURECOLLECTION_BLOB_H
