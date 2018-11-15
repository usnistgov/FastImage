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

/// @file ViewRequestData.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/31/17
/// @brief Data Representing a View Request

#ifndef FASTIMAGE_VIEWREQUESTDATA_H
#define FASTIMAGE_VIEWREQUESTDATA_H

#include <htgs/api/IData.hpp>
#include <ostream>
#include <cmath>

namespace fi {
/// \namespace fi FastImage namespace

/**
 * @class ViewRequestData ViewRequestData.h <FastImage/data/ViewRequestData.h>
 * @brief Data representing a view request
 * @details FastImage will use this object to request a view.
 * It will be sent to a ViewLoader. The view Loader will then use it to generate
 * TileRequestData.
 *
 * @tparam UserType Pixel Type asked by the end user
 *
 */
template<typename UserType>
class ViewRequestData : public htgs::IData {
 public:

  /// \brief Construct a  ViewRequestData
  /// \param indexTileRow Row index tile asked
  /// \param indexTileCol Col index tile asked
  /// \param numTilesHeight Number of tiles in height
  /// \param numTilesWidth Number of tiles in width
  /// \param radius View Radius
  /// \param tileHeight Tile height in px
  /// \param tileWidth Tile width in px
  /// \param imageHeight Image Height in px
  /// \param imageWidth Image Width in px
  /// \param level Pyramid level
  ViewRequestData(
      uint32_t indexTileRow, uint32_t indexTileCol,
      uint32_t numTilesHeight, uint32_t numTilesWidth, uint32_t radius,
      uint32_t tileHeight, uint32_t tileWidth,
      uint32_t imageHeight, uint32_t imageWidth, uint32_t level)
      : _imageWidth(imageWidth),
        _imageHeight(imageHeight),
        _tileHeight(tileHeight),
        _tileWidth(tileWidth),
        _radius(radius),
        _indexRowCenterTile(indexTileRow),
        _indexColCenterTile(indexTileCol),
        _level(level) {
    // View Size
    _viewHeight = tileHeight + 2 * radius;
    _viewWidth = tileWidth + 2 * radius;
    // Top left central tile pixel
    _minRowCentralTile = _indexRowCenterTile * tileHeight;
    _minColCentralTile = _indexColCenterTile * tileWidth;
    // Index of the tile overlapped by the view.
    _indexRowMinTile = (uint32_t) std::max(
        (int32_t) _indexRowCenterTile
            - (int32_t) ceil((double) radius / tileHeight),
        (int32_t) 0);
    _indexColMinTile = (uint32_t) std::max(
        (int32_t) _indexColCenterTile
            - (int32_t) ceil((double) radius / tileWidth),
        (int32_t) 0);
    _indexRowMaxTile = std::min(
        _indexRowCenterTile + (int32_t) ceil((double) radius / tileHeight) + 1,
        numTilesHeight);
    _indexColMaxTile = std::min(
        _indexColCenterTile + (int32_t) ceil((double) radius / tileWidth) + 1,
        numTilesWidth);

    // Position of the lines / columns to copy from the file
    _minRowFile = (uint32_t) std::max((int32_t) (_minRowCentralTile - radius),
                                      (int32_t) 0);
    _maxRowFile =
        std::min((_indexRowCenterTile + 1) * tileHeight + radius, imageHeight);
    _minColFile = (uint32_t) std::max((int32_t) (_minColCentralTile - radius),
                                      (int32_t) 0);
    _maxColFile =
        std::min((_indexColCenterTile + 1) * tileWidth + radius, imageWidth);

    // Count of the lines / columns to copy from the file
    _rowFilledFromFile = _maxRowFile - _minRowFile;
    _colFilledFromFile = _maxColFile - _minColFile;

    // Number of pixels to fill with ghost values
    _topFill = (int32_t) (_minRowCentralTile - radius) < 0 ? radius
        - _minRowCentralTile : 0;
    _leftFill = (int32_t) (_minColCentralTile - radius) < 0 ? radius
        - _minColCentralTile : 0;
    _bottomFill =
        (_topFill + _rowFilledFromFile) < _viewHeight ? _viewHeight
            - (_topFill + _rowFilledFromFile) : 0;
    _rightFill =
        (_leftFill + _colFilledFromFile) < _viewWidth ? _viewWidth
            - (_leftFill + _colFilledFromFile) : 0;
    _numberTilesToLoad = (_indexRowMaxTile - _indexRowMinTile)
        * (_indexColMaxTile - _indexColMinTile);
  }

  /// \brief Get the image width in px
  /// \return The image width in px
  uint32_t getImageWidth() const { return _imageWidth; }

  /// \brief Get the image height in px
  /// \return The image height in px
  uint32_t getImageHeight() const { return _imageHeight; }

  /// \brief Get tile height
  /// \return Tile height
  uint32_t getTileHeight() const { return _tileHeight; }

  /// \brief Get tile width
  /// \return Tile width
  uint32_t getTileWidth() const { return _tileWidth; }

  /// \brief Get radius
  /// \return Radius
  uint32_t getRadius() const { return _radius; }

  /// \brief Get view height
  /// \return View height
  uint32_t getViewHeight() const { return _viewHeight; }

  /// \brief Get view width
  /// \return View width
  uint32_t getViewWidth() const { return _viewWidth; }

  /// \brief Get number of tiles to load from the file
  /// \return Number of tiles to load from the file
  uint32_t getNumberTilesToLoad() const { return _numberTilesToLoad; }

  /// \brief Get the row index of the upper left tile
  /// \return The row index of the upper left tile
  uint32_t getIndexRowMinTile() const { return _indexRowMinTile; }

  /// \brief Get the column index of the upper left tile
  /// \return The column index of the upper left tile
  uint32_t getIndexColMinTile() const { return _indexColMinTile; }

  /// \brief Get the row index of the center tile
  /// \return The row index of the center tile
  uint32_t getIndexRowCenterTile() const { return _indexRowCenterTile; }

  /// \brief Get the column index of the center tile
  /// \return The column index of the center tile
  uint32_t getIndexColCenterTile() const { return _indexColCenterTile; }

  /// \brief Get the row index of the bottom right tile
  /// \return The row index of the bottom right tile
  uint32_t getIndexRowMaxTile() const { return _indexRowMaxTile; }

  /// \brief Get the column index of the bottom right tile
  /// \return The column index of the bottom right tile
  uint32_t getIndexColMaxTile() const { return _indexColMaxTile; }

  /// \brief Get file minimum row
  /// \return File minimum row
  uint32_t getMinRowFile() const { return _minRowFile; }

  /// \brief Get file maximum row
  /// \return File maximum row
  uint32_t getMaxRowFile() const { return _maxRowFile; }

  /// \brief Get file minimum column
  /// \return File minimum column
  uint32_t getMinColFile() const { return _minColFile; }

  /// \brief Get file maximum column
  /// \return File maximum column
  uint32_t getMaxColFile() const { return _maxColFile; }

  /// \brief Get top rows to fill with ghost data
  /// \return Top rows to fill with ghost data
  uint32_t getTopFill() const { return _topFill; }

  /// \brief Get left columns to fill with ghost data
  /// \return Left columns to fill with ghost data
  uint32_t getLeftFill() const { return _leftFill; }

  /// \brief Get bottom rows to fill with ghost data
  /// \return Bottom rows to fill with ghost data
  uint32_t getBottomFill() const { return _bottomFill; }

  /// \brief Get right columns to fill with ghost data
  /// \return Right columns to fill with ghost data
  uint32_t getRightFill() const { return _rightFill; }

  /// \brief Get pyramid level
  /// \return Pyramid level
  uint32_t getLevel() const { return _level; }

  /// \brief Output stream operator
  /// \param os output stream
  /// \param data data to print
  /// \return output stream with data printed
  friend std::ostream &operator<<(std::ostream &os,
                                  const ViewRequestData &data) {
    os << "ViewRequestData[_indexRowTileAsked: "
       << data.getIndexRowMinTile() << "," << data.getIndexRowCenterTile()
       << "," << data.getIndexRowMaxTile()
       << "/_indexColTileAsked: "
       << data.getIndexColMinTile() << "," << data.getIndexColCenterTile()
       << "," << data.getIndexColMaxTile()
       << "]";
    return os;
  }
 private:
  uint32_t
  // Image size
      _imageWidth,            ///< Image width in px
      _imageHeight,           ///< Image height in px
  // Tile size
      _tileHeight,            ///< Tile Height
      _tileWidth,             ///< Tile Width
  // Radius
      _radius,                ///< Number of pixel surrounding the center tile
  // View Size
      _viewHeight,            ///< View Height in px
      _viewWidth,             ///< View Width in px
      _numberTilesToLoad,     ///< Number of tiles to load
  // Central Tile
      _minRowCentralTile,     ///< Minimum row central tile
      _minColCentralTile,     ///< Minimum col central tile
  // Index Tile into view
      _indexRowMinTile,       ///< Row index minimum tile
      _indexColMinTile,       ///< Column index minimum tile
      _indexRowCenterTile,    ///< Row index central tile
      _indexColCenterTile,    ///< Column index central tile
      _indexRowMaxTile,       ///< Row index maximum tile
      _indexColMaxTile,       ///< Col index maximum tile
  // Position View in File
      _minRowFile,            ///< File minimum row
      _maxRowFile,            ///< File maximum row
      _minColFile,            ///< File minimum col
      _maxColFile,            ///< File maximum col
  // Filling Ghost Region
      _rowFilledFromFile,     ///< Number of rows filled from the file
      _colFilledFromFile,     ///< Number of columns filled from the file
      _topFill,               ///< Top rows to fill with ghost data
      _leftFill,              ///< Left columns to fill with ghost data
      _bottomFill,            ///< Bottom rows to fill with ghost data
      _rightFill,             ///< Right columns to fill with ghost data
      _level;                 ///< Image Pyramid level
};
}

#endif //FASTIMAGE_VIEWREQUESTDATA_H
