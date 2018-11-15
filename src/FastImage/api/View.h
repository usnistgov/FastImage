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

/// @file View.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/25/17
/// @brief Image's region


#ifndef FASTIMAGE_VIEWDATA_H
#define FASTIMAGE_VIEWDATA_H

#include <sstream>
#include <iomanip>
#include <vector>
#include "FastImage/data/CachedTile.h"
#include "../data/DataType.h"
#include "FastImage/data/ViewRequestData.h"
#include "../object/FigCache.h"
#include "../exception/FastImageException.h"

/// \namespace fi FastImage namespace
namespace fi {

/**
 * @class View View.h <FastImage/api/View.h>
 * @brief View used by FastImage to represent image's region.
 * @details View data class, contains the tiles loaded by the Fast-Image graph.
 * The view is defined by a central tile, and the pixels surrounding it. The
 * views will be pre-allocated by the memory manager. The pixels are located by
 * their local coordinate where the position (0/0) is in the upper left of the
 * central tile.
 *
 * For example to get once all the pixel of the image, the radius is set to 0,
 * and it can be used:
 *
 * @code
 * for (int32_t r = 0; r < view->getTileHeight(); ++r) {
 *   for (int32_t c = 0; c < view->getTileWidth(); ++c) {
 *     // Get the pixel from it's coordinate
 *     pixValue = view->getPixel(r, c);
 *     if (!std::isnan(pixValue)) {
 *       ...
 *     }
 *   }
 * }
 * @endcode
 *
 * @tparam UserType Pixel Type asked by the end user
 */
template<typename UserType>
class View : public htgs::IData {
 public:
  /// \brief Create the View, allocate the array of pixel.
  /// \param row Number of pixel in a view
  /// \param col Number of pixel in a view
  View(const uint32_t &row, const uint32_t &col)
  : _data(new UserType[row * col]), _viewHeight(row), _viewWidth(col) {}

  /// \brief View destructor, deallocate the array of pixel.
  ~View() override { delete[] _data; }

  /// \brief Get view width in px
  /// \return View width in px
  uint32_t getViewWidth() const { return _viewWidth; }

  /// \brief Get view height in px
  /// \return View height in px
  uint32_t getViewHeight() const { return _viewHeight; }

  /// \brief Get Tile width of pixels coming from the image
  /// \note Differ from the tile width from the fast image which is the tile
  /// width from the file
  /// \return Tile Width of pixels coming from the file
  int32_t getTileWidth() const { return _maxColCenterTileLocal; }

  /// \brief Get Tile height of pixels coming from the file
  /// \note Differ from the tile height from the fast image which is the tile
  /// height from the file
  /// \return Tile height of pixels coming from the file
  int32_t getTileHeight() const { return _maxRowCenterTileLocal; }

  /// \brief Get row grid index of the view's tile
  /// \return Row grid index of the view's tile
  uint32_t getRow() const {
    return this->_viewRequestData->getIndexRowCenterTile();
  }

  /// \brief Get column grid index of the view's tile
  /// \return Column grid index of the view's tile
  uint32_t getCol() const {
    return this->_viewRequestData->getIndexColCenterTile();
  }

  /// \brief Get the View Radius
  /// \return View Radius
  uint32_t getRadius() const { return _viewRequestData->getRadius(); }

  /// \brief Get Global Tile X Offset
  /// \note Only used to map DO NOT USE with GetPixel
  /// \return Global Tile X Offset
  uint32_t getGlobalXOffset() const {
    return getCol() * this->_viewRequestData->getTileWidth();
  }

  /// \brief Get Global Tile Y Offset
  /// \note Only used to map DO NOT USE with GetPixel
  /// \return Global Tile Y Offset
  uint32_t getGlobalYOffset() const {
    return getRow() * this->_viewRequestData->getTileHeight();
  }

  /// \brief Get the pixel value from it local coordinate
  /// \param rowAsked Row coordinate's pixel
  /// \param colAsked Column coordinate's pixel
  /// \note Shout be only use with local coordinates
  /// \return The pixel value
  UserType getPixel(const int32_t &rowAsked, const int32_t &colAsked) const {
    assert(isLocalCoordinateCorrect(rowAsked, colAsked));
    return (_data[(rowAsked + getRadius()) * _viewWidth
        + (colAsked + getRadius())]);
  }

  /// \brief Set a pixel in the view
  /// \param rowAsked Pixel's row
  /// \param colAsked Pixel's col
  /// \param value value to set
  void setPixel(const int32_t &rowAsked,
                const int32_t &colAsked,
                const UserType &value) {
    assert(isLocalCoordinateCorrect(rowAsked, colAsked));
    _data[(rowAsked + getRadius()) * _viewWidth + (colAsked + getRadius())] =
        value;
  }

  /// \brief Get the pointer to the central tile
  /// \return Pointer to the central tile
  UserType *getPointerTile() {
    return (_data + getRadius() * (_viewWidth + 1));
  }

  /// \brief Get the pointer to the view
  /// \return The pointer to the view
  UserType *getData() const { return _data; }

  /// \brief Get the pyramid level the view come from
  /// \return Pyramid level
  uint32_t getPyramidLevel() const { return _viewRequestData->getLevel(); }

  /// \brief Get the leading dimension
  /// \return Leading dimension
  uint32_t getLeadingDimension() { return getViewWidth(); }

  /// \brief Initializes the view with the information contained in the view request
  /// and the tile size
  /// \param viewRequest View request information to initialize the view
  /// \param fillingType Filling used to complete the ghost region
  void init(
      const std::shared_ptr<fi::ViewRequestData<UserType>> &viewRequest,
      FillingType fillingType = FillingType::FILL) {

    auto
        tileHeight = viewRequest->getTileHeight(),
        tileWidth = viewRequest->getTileWidth();

    this->_viewRequestData = viewRequest;
    _minRowCenterTileGlobal = viewRequest->getIndexRowCenterTile() * tileHeight;
    _minColCenterTileGlobal = viewRequest->getIndexColCenterTile() * tileWidth;
    _maxRowCenterTileGlobal = std::min(viewRequest->getImageHeight(),
                                       _minRowCenterTileGlobal + tileHeight);
    _maxColCenterTileGlobal = std::min(viewRequest->getImageWidth(),
                                       _minColCenterTileGlobal + tileWidth);
    _maxColCenterTileLocal = _maxColCenterTileGlobal
        - viewRequest->getIndexColCenterTile() * tileWidth;
    _maxRowCenterTileLocal = _maxRowCenterTileGlobal
        - viewRequest->getIndexRowCenterTile() * tileHeight;
  }

  /// \brief Output operator stream to print a view
  /// \param os Stream to put the view information
  /// \param data The ViewData to print
  /// \return the output stream with the information
  /// \note Used for debugging only
  friend std::ostream &operator<<(std::ostream &os, const View &data) {
    os << "View:" << std::endl << "    Index: " << data.getRow() << "/"
       << data.getCol() << std::endl;

    for (int32_t row = -data.getRadius(); row < 0; ++row) {
      for (int32_t col = -data.getRadius(); col < 0; ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << "   ";
      for (int32_t col = 0; col < (int32_t) data.getTileWidth(); ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << "   ";
      for (auto col = (int32_t) data.getTileWidth();
           col < (int32_t) data.getViewWidth() - (int32_t) data.getRadius();
           ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << std::endl;
    }

    os << std::endl;

    for (int32_t row = 0; row < data.getTileHeight(); ++row) {
      for (int32_t col = -data.getRadius(); col < 0; ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << "   ";
      for (int32_t col = 0; col < data.getTileWidth(); ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << "   ";
      for (auto col = (int32_t) data.getTileWidth();
           col < (int32_t) data.getViewWidth() - (int32_t) data.getRadius();
           ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << std::endl;
    }

    os << std::endl;

    for (auto row = (int32_t) data.getTileHeight();
         row < (int32_t) data.getViewHeight() - (int32_t) data.getRadius();
         ++row) {
      for (int32_t col = -data.getRadius(); col < 0; ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << "   ";
      for (int32_t col = 0; col < data.getTileWidth(); ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << "   ";
      for (auto col = (int32_t) data.getTileWidth();
           col < (int32_t) data.getViewWidth() - (int32_t) data.getRadius();
           ++col) {
        os << std::setw(3) << (int) data.getPixel(row, col) << " ";
      }
      os << std::endl;
    }

    os << std::endl;

    return os;
  }

 private:
  UserType *
      _data;                  ///< Augmented Tile, Tile w/ a ghost region

  std::shared_ptr<fi::ViewRequestData<UserType>>
      _viewRequestData;       ///< ViewRequest creating the view

  uint32_t
      _viewHeight,            ///< Tile height in pixel
      _viewWidth,             ///< Tile width in pixel

      _minRowCenterTileGlobal
      {},          ///< Row minimum in the central tile in global coordinate
      _minColCenterTileGlobal
      {},          ///< Column minimum in the central tile in global coordinate
      _maxRowCenterTileGlobal
      {},          ///< Row maximum in the central tile in global coordinate
      _maxColCenterTileGlobal
      {};          ///< Column maximum in the central tile in global coordinate

  int32_t
      _maxRowCenterTileLocal
      {},           ///< Row maximum in the central tile in local coordinate
      _maxColCenterTileLocal
      {};           ///< Column maximum in the central tile in local coordinate

  /// \brief Test if the local coordinate is in the view
  /// \param rowAsked Row asked in picture coordinate
  /// \param colAsked Global asked in picture coordinate
  /// \return True if the coordinates are in the view, else false
  bool isLocalCoordinateCorrect(const int32_t &rowAsked,
                                const int32_t &colAsked) const {
    return rowAsked >= (int32_t) -getRadius() &&
      rowAsked < (int32_t) this->_viewRequestData->getViewHeight()
          - (int32_t) getRadius() &&
      colAsked >= (int32_t) -getRadius() &&
      colAsked < (int32_t) this->_viewRequestData->getViewWidth()
          - (int32_t) getRadius();
  }
};
}

#endif //FASTIMAGE_VIEWDATA_H
